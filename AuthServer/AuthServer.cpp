#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <timers.hpp>
#include <iostream>

// Config
config_file *AuthServer::auth_config;
config_file *AuthServer::database_config;

struct AuthServer::login_config AuthServer::config;

// Network
tcp_server *AuthServer::server;

// Database
soci::session *AuthServer::database;
AccountDB *AuthServer::accounts;

// Interconnection
std::map<int, struct CharServerConnection> AuthServer::servers;

void AuthServer::run()
{
	boost::asio::io_service io_service;

	// Read Config Files
	try
	{
		auth_config = new config_file("./Config/authserver.conf");
		{
			config.network_bindip = auth_config->read<string>("network.bindip", "0.0.0.0");
			config.network_bindport = auth_config->read<unsigned short>("network.bindport", 6900);
		}
		ShowStatus("Finished reading authserver.conf.\n");

		database_config = new config_file("./Config/database.conf");
		{
			// Validate something?
		}
		ShowStatus("Finished reading database.conf.\n");
	}
	catch (config_file::file_not_found *fnf)
	{
		ShowFatalError("Config file not found: %s.\n", fnf->filename);
		return;
	}

	TimerManager::Initialize(&io_service);

	// Initialize Database System
	{
		ShowInfo("Opening connection to database...\n");

		database = database_helper::get_session(database_config);
		accounts = new AccountDB(database);

		ShowSQL("Successfully opened database connection.\n");
	}

	// Initialize Network System
	{
		boost::system::error_code err;
		address_v4 bindip = address_v4::from_string(config.network_bindip, err);

		if (err)
		{
			ShowFatalError("%s\n", err.message().c_str());
			return;
		}

		server = new tcp_server(io_service, (address)bindip, config.network_bindport);
		server->set_default_parser(AuthServer::parse_from_client);

		ShowStatus("AuthServer is ready and listening on %s:%d.\n", config.network_bindip.c_str(), config.network_bindport);
	}

	// Run IO service service and start pooling events
	io_service.run();
}

int AuthServer::parse_from_char(tcp_connection::pointer cl)
{
	AuthSessionData *asd = ((AuthSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		ShowInfo("Closed connection from CharServer '"CL_WHITE"%s"CL_RESET"'.\n", servers[asd->account_id].name);
		cl->do_close();

		servers.erase(asd->account_id);
		// TODO: Set Players Offline

		return 0;
	}



	return 0;
}

int AuthServer::parse_from_client(tcp_connection::pointer cl)
{
	AuthSessionData *asd = ((AuthSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	if (asd == NULL)
	{
		asd = new AuthSessionData();
		memset(asd, 0, sizeof(asd));

		asd->cl = cl;

		cl->set_data((char*)asd);
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{
			// request client login (raw password)
		case 0x0064: // S 0064 <version>.L <username>.24B <password>.24B <clienttype>.B
		case 0x0277: // S 0277 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B
		case 0x02b0: // S 02b0 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B <g_isGravityID>.B
			// request client login (md5-hashed password)
		case 0x01dd: // S 01dd <version>.L <username>.24B <password hash>.16B <clienttype>.B
		case 0x01fa: // S 01fa <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.B(index of the connection in the clientinfo file (+10 if the command-line contains "pc"))
		case 0x027c: // S 027c <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.13B(junk)
			// token-based login
		case 0x0825: // S 0825 <packetsize>.W <version>.L <clienttype>.B <userid>.24B <password>.27B <mac>.17B <ip>.15B <token>.(packetsize - 0x5C)B
			{
				// TODO: Implement token based login
				size_t packet_len = RFIFOREST(cl);
				unsigned int version;
				char username[NAME_LENGTH];
				char password[NAME_LENGTH];
				unsigned char passhash[16];
				unsigned char clienttype;
				bool israwpass = (cmd == 0x0064 || cmd == 0x0277 || cmd == 0x02b0 || cmd == 0x0825);

				if((cmd == 0x0064 && packet_len < 55)
					||  (cmd == 0x0277 && packet_len < 84)
					||  (cmd == 0x02b0 && packet_len < 85)
					||  (cmd == 0x01dd && packet_len < 47)
					||  (cmd == 0x01fa && packet_len < 48)
					||  (cmd == 0x027c && packet_len < 60) 
					||  (cmd == 0x0825 && (packet_len < 4 || packet_len < RFIFOW(cl, 2)))
					)
					return 0;

				// Token-based authentication model by Shinryo
				if(cmd == 0x0825)
				{	
					char *accname = (char *)RFIFOP(cl, 9);
					char *token = (char *)RFIFOP(cl, 0x5C);
					size_t uAccLen = strlen(accname);
					size_t uTokenLen = packet_len - 0x5C;

					version = RFIFOL(cl,4);

					if(uAccLen > NAME_LENGTH - 1 || uAccLen <= 0 || uTokenLen != 32)
					{
						// TODO: Failed
						return 0;
					}

					clienttype = RFIFOB(cl, 8);
				}
				else
				{
					version = RFIFOL(cl,2);
					strncpy(username, (const char*)RFIFOP(cl,6), NAME_LENGTH);

					if( israwpass )
					{
						strncpy(password, (const char*)RFIFOP(cl,30), NAME_LENGTH);
						clienttype = RFIFOB(cl,54);
					}
					else
					{
						memcpy(passhash, RFIFOP(cl,30), 16);
						clienttype = RFIFOB(cl,46);
					}
				}			

				cl->skip(RFIFOREST(cl)); // assume no other packet was sent

				asd->clienttype = clienttype;
				asd->version = version;
				strncpy(asd->username, username, NAME_LENGTH);

				ShowStatus("Request for connection of %s from %s.\n", asd->username, cl->socket().remote_endpoint().address().to_string().c_str());
				if (cmd == 0x0825)
				{
					// TODO: Add support to token-based login

					asd->type = auth_token;
				}
				else if (israwpass)
				{
					strncpy(asd->password, password, NAME_LENGTH);

					// TODO: Add support to MD5 encoded passwords
					/*if(login_config.use_md5_passwds)
						MD5_String(sd->passwd, sd->passwd);*/

					asd->type = auth_raw;
				}
				else
				{
					// TODO: Add support to MD5 encoded passwords
					//bin2hex(sd->passwd, passhash, 16); // raw binary data here!

					asd->type = auth_md5;
				}

				int result = authenticate(asd);

				if (result != -1)
					send_auth_err(asd, result);
				else
					send_auth_ok(asd);
			}
			break;
			// CharServer login
		case 0x3000: // S 3000 <login>.24B <password>.24B <display name>.20B
			if (RFIFOREST(cl) < 76)
				return 0;
			{
				char server_name[20];
				address_v4 addr;
				unsigned short port;

				strncpy(asd->username, (char*)RFIFOP(cl, 2), NAME_LENGTH);
				strncpy(asd->password, (char*)RFIFOP(cl, 26), NAME_LENGTH);
				strncpy(server_name, (char*)RFIFOP(cl, 50), 20);
				
				addr = address_v4(ntohl(RFIFOL(cl, 70)));
				port = ntohs(RFIFOW(cl, 74));

				cl->skip(76);

				asd->type = auth_raw;
				
				int result = authenticate(asd);
				if (result == -1 && asd->sex == 'S' && !servers.count(asd->account_id))
				{
					servers[asd->account_id].addr = addr;
					servers[asd->account_id].port = port;
					servers[asd->account_id].account_id = asd->account_id;
					servers[asd->account_id].cl = cl;

					strncpy(servers[asd->account_id].name, server_name, 20);

					cl->flags.server = 1;
					cl->realloc_fifo(FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
					cl->set_parser(&AuthServer::parse_from_char);

					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = 0x3000;
					WFIFOB(cl,2) = 0;
					cl->send_buffer(3);
				}
				else
				{
					ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);
					
					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = 0x3000;
					WFIFOB(cl,2) = 3;
					cl->send_buffer(3);
				}
			}
			break;
		default:
			ShowWarning("Unknown packet 0x%x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_YELLOW"       Equipe Cronus de Desenvolvimento Apresenta        "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      _________                                          "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      \\_   ___ \\_______  ____   ____  __ __  ______      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      /    \\  \\/\\_  __ \\/  _ \\ /    \\|  |  \\/  ___/      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      \\     \\____|  | \\(  <_> )   |  \\  |  /\\___ \\       "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"       \\______  /|__|   \\____/|___|  /____//____  >      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"              \\/                   \\/           \\/       "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                         "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_RED"                         Cronus++                        "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                  www.cronus-emulator.com                "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                         "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n\n");

	AuthServer::run();

	getchar();
	return 0;
}
