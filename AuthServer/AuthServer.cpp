#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>

#include <md5.hpp>
#include <strfuncs.hpp>
#include <boost/foreach.hpp>


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
AuthServer::char_server_db AuthServer::servers;

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
			config.auth_use_md5 = auth_config->read<bool>("auth.usemd5", false);
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

		try
		{
			database = database_helper::get_session(database_config);
		}
		catch (soci::soci_error err)
		{
			ShowFatalError("Error opening database connection: %s\n", err.what());
			return;
		}

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

void AuthServer::char_sendallwos(int cs, unsigned char *buf, size_t len)
{			
	BOOST_FOREACH(char_server_db::value_type &pair, servers)
	{
		if (pair.second.account_id != cs)
		{
			WFIFOHEAD(pair.second.cl, len);
			memcpy(WFIFOP(pair.second.cl, 0), buf, len);
			pair.second.cl->send_buffer(len);
		}
	}
}

int AuthServer::parse_from_char(tcp_connection::pointer cl)
{
	AuthSessionData *asd = ((AuthSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		if (asd)
		{
			ShowInfo("Closed connection from CharServer '"CL_WHITE"%s"CL_RESET"'.\n", servers[asd->account_id].name);

			servers.erase(asd->account_id);
			
			BOOST_FOREACH(online_account_db::value_type &pair, online_accounts)
			{
				if (pair.second.charserver == asd->account_id)
				{
					if (pair.second.disconnect_timer)
						TimerManager::FreeTimer(pair.second.disconnect_timer);

					pair.second.disconnect_timer = TimerManager::CreateStartTimer(500, false, boost::bind(&AuthServer::disconnect_user, _1, pair.first));
				}
			}

			delete asd;
		}

		cl->do_close();
		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{
		case INTER_CA_REQ_ACC_DATA:
			if(RFIFOREST(cl) < 10)
				return 0;
			{
				Account acc;
				time_t expiration_time = 0;
				char email[40] = "";
				int gmlevel = 0;
				char birthdate[11] = "";
				int rid = RFIFOL(cl,6);

				int account_id = RFIFOL(cl,2);
				cl->skip(10);

				if (!accounts->load_account(account_id, acc))
				{
					expiration_time = acc.expiration_time;
					strncpy(email, acc.email.c_str(), 40);
					gmlevel = acc.level;
					strncpy(birthdate, acc.birthdate.c_str(), 11);
				}

				WFIFOHEAD(cl,66);
				WFIFOW(cl,0) = INTER_AC_REQ_ACC_DATA_REPLY;
				WFIFOL(cl,2) = account_id;
				strncpy((char*)WFIFOP(cl,6), email, 40);
				WFIFOL(cl,46) = (unsigned int)expiration_time;
				WFIFOB(cl,50) = gmlevel;
				strncpy((char*)WFIFOP(cl,51), birthdate, 10+1);
				WFIFOL(cl,62) = rid;
				cl->send_buffer(66);
			}
		case INTER_CA_SET_ACC_ON:
			if(RFIFOREST(cl) < 6)
				return 0;
			add_online_user(asd->account_id, RFIFOL(cl,2));
			cl->skip(6);
			break;
		case INTER_CA_SET_ACC_OFF:
			if(RFIFOREST(cl) < 6)
				return 0;
			set_acc_offline(RFIFOL(cl,2));
			cl->skip(6);
			break;
		case INTER_CA_AUTH:
			if(RFIFOREST(cl) < 23)
				return 0;
			{
				int account_id = RFIFOL(cl,2);
				unsigned int login_id1 = RFIFOL(cl,6);
				unsigned int login_id2 = RFIFOL(cl,10);
				unsigned char sex = RFIFOB(cl,14);
				int request_id = RFIFOL(cl,19);
				cl->skip(23);

				if (auth_nodes.count(account_id) &&
					auth_nodes[account_id].login_id1  == login_id1 &&
					auth_nodes[account_id].login_id2  == login_id2 &&
					auth_nodes[account_id].sex        == sex_num2str(sex))
				{
					WFIFOHEAD(cl,25);
					WFIFOW(cl,0) = INTER_AC_AUTH_REPLY;
					WFIFOL(cl,2) = account_id;
					WFIFOL(cl,6) = login_id1;
					WFIFOL(cl,10) = login_id2;
					WFIFOB(cl,14) = sex;
					WFIFOB(cl,15) = 0;
					WFIFOL(cl,16) = request_id;
					WFIFOL(cl,20) = auth_nodes[account_id].version;
					WFIFOB(cl,24) = auth_nodes[account_id].clienttype;
					cl->send_buffer(25);

					auth_nodes.erase(account_id);
				}
				else
				{
					WFIFOHEAD(cl,25);
					WFIFOW(cl,0) = INTER_AC_AUTH_REPLY;
					WFIFOL(cl,2) = account_id;
					WFIFOL(cl,6) = login_id1;
					WFIFOL(cl,10) = login_id2;
					WFIFOB(cl,14) = sex;
					WFIFOB(cl,15) = 1;
					WFIFOL(cl,16) = request_id;
					WFIFOL(cl,20) = 0;
					WFIFOB(cl,24) = 0;
					cl->send_buffer(25);
				}
			}
			break;
		default:
			ShowWarning("Unknown packet 0x%x sent from CharServer '%s', closing connection.\n", cmd, servers[asd->account_id].name);
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

int AuthServer::parse_from_client(tcp_connection::pointer cl)
{
	AuthSessionData *asd = ((AuthSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		if (asd)
			delete asd;

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
		case HEADER_CA_LOGIN:			// S 0064 <version>.L <username>.24B <password>.24B <clienttype>.B
		case HEADER_CA_LOGIN_PCBANG:	// S 0277 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B
		case HEADER_CA_LOGIN_CHANNEL:	// S 02b0 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B <g_isGravityID>.B
			// request client login (md5-hashed password)
		case HEADER_CA_LOGIN2:			// S 01dd <version>.L <username>.24B <password hash>.16B <clienttype>.B
		case HEADER_CA_LOGIN3:			// S 01fa <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.B(index of the connection in the clientinfo file (+10 if the command-line contains "pc"))
		case HEADER_CA_LOGIN4:			// S 027c <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.13B(junk)
			// token-based login
		case HEADER_CA_LOGIN_TOKEN:		// S 0825 <packetsize>.W <version>.L <clienttype>.B <userid>.24B <password>.27B <mac>.17B <ip>.15B <token>.(packetsize - 0x5C)B
			{
				// TODO: Implement token based login
				size_t packet_len = RFIFOREST(cl);
				unsigned int version;
				char username[NAME_LENGTH];
				char password[NAME_LENGTH];
				unsigned char passhash[16];
				unsigned char clienttype;
				bool israwpass = (cmd == HEADER_CA_LOGIN || cmd == HEADER_CA_LOGIN_PCBANG || cmd == HEADER_CA_LOGIN_CHANNEL || cmd == HEADER_CA_LOGIN_TOKEN);

				if((cmd == HEADER_CA_LOGIN && packet_len < 55)
					||  (cmd == HEADER_CA_LOGIN_PCBANG && packet_len < 84)
					||  (cmd == HEADER_CA_LOGIN_CHANNEL && packet_len < 85)
					||  (cmd == HEADER_CA_LOGIN2 && packet_len < 47)
					||  (cmd == HEADER_CA_LOGIN3 && packet_len < 48)
					||  (cmd == HEADER_CA_LOGIN4 && packet_len < 60) 
					||  (cmd == HEADER_CA_LOGIN_TOKEN && (packet_len < 4 || packet_len < RFIFOW(cl, 2)))
					)
					return 0;

				// Token-based authentication model by Shinryo
				if(cmd == HEADER_CA_LOGIN_TOKEN)
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
				if (cmd == HEADER_CA_LOGIN_TOKEN)
				{
					// TODO: Add support to token-based login

					asd->type = auth_token;
				}
				else if (israwpass)
				{
					strncpy(asd->password, password, NAME_LENGTH);

					if(config.auth_use_md5)
						md5(asd->password);

					asd->type = auth_raw;
				}
				else
				{
					bin2hex(asd->password, passhash, 16); // raw binary data here!
					asd->type = auth_md5;
				}

				int result = authenticate(asd);

				if (result != -1)
					send_auth_err(asd, result);
				else
					send_auth_ok(asd);
			}
			break;
        
		// Md5 Login
		case HEADER_CA_REQ_HASH:
			{
				memset(asd->md5key, '\0', sizeof(asd->md5key));
				asd->md5keylen = (boost::uint16_t)(12 + rand() % 4);
				MD5_Salt(asd->md5keylen, asd->md5key);

				WFIFOHEAD(cl, 4 + asd->md5keylen);
				WFIFOW(cl,0) = HEADER_AC_ACK_HASH;
				WFIFOW(cl,2) = 4 + asd->md5keylen;
				memcpy(WFIFOP(cl,4), asd->md5key, asd->md5keylen);
				cl->send_buffer(WFIFOW(cl,2));
				cl->skip(2);
				
			}
			break;
		
		//nProtect GameGuard Challenge
		case HEADER_CA_REQ_GAME_GUARD_CHECK:
			{
				WFIFOHEAD(cl,3);
				WFIFOW(cl,0) = HEADER_AC_ACK_GAME_GUARD;
				WFIFOB(cl,2) = ((asd->gameguardChallenged)?(2):(1));
				asd->gameguardChallenged = true;
				cl->send_buffer(3);
				cl->skip(2);
			}
			
		// CharServer login
		case INTER_CA_LOGIN: // S 3000 <login>.24B <password>.24B <display name>.20B
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
					WFIFOW(cl,0) = INTER_AC_LOGIN_REPLY;
					WFIFOB(cl,2) = 0;
					cl->send_buffer(3);
				}
				else
				{
					ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);
					
					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = INTER_AC_LOGIN_REPLY;
					WFIFOB(cl,2) = 3;
					cl->send_buffer(3);
				}
			}
			break;
		default:
			ShowWarning("Unknown packet 0x%04x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

void AuthServer::add_online_user(int id, int accid)
{
	online_accounts[accid].charserver = id;

	if (online_accounts[accid].disconnect_timer)
		TimerManager::FreeTimer(online_accounts[accid].disconnect_timer);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	core_display_title();

	AuthServer::run();

	getchar();
	return 0;
}
