/*============ Cronus++ developer team presents: ==========*
*	______ _           _           _           _		   *
*	|  ___(_)         | |         | |         | |		   *
*	| |_   _ _ __ ___ | |__  _   _| |_   _____| |_ _ __    *
*	|  _| | | '_ ` _ \| '_ \| | | | \ \ / / _ \ __| '__|   *
*	| |   | | | | | | | |_) | |_| | |\ V /  __/ |_| |      *
*	\_|   |_|_| |_| |_|_.__/ \__,_|_| \_/ \___|\__|_|      *
* -------------------------------------------------------- *
*               An Ragnarok Online Emulator                *
* -------------------------------------------------------- *
*                Licenced under GNU GPL v3                 *
* -------------------------------------------------------- *
*					 Character Server					   *
* ======================================================== */

#include "CharServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <boost/thread.hpp>
#include <ragnarok.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>
#include <boost/foreach.hpp>

// Login InterConn
tcp_connection::pointer CharServer::auth_conn;

// Config
config_file *CharServer::auth_config;
config_file *CharServer::database_config;

struct CharServer::login_config CharServer::config;

// Network
boost::asio::io_service *CharServer::io_service;
tcp_server *CharServer::server;

// Database
soci::session *CharServer::database;

// Auth
CharServer::auth_node_db CharServer::auth_nodes;
CharServer::online_account_db CharServer::online_chars;
bool CharServer::auth_conn_ok;

/*==============================================================*
* Function:	Start Char Server									*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Start the char-server and load the confs         *
**==============================================================*/
void CharServer::run()
{
	io_service = new boost::asio::io_service();

	// Read Config Files
	try
	{
		auth_config = new config_file("./Config/charserver.conf");
		{
			config.server_name = auth_config->read<string>("server.name", "Cronus++");

			config.network_bindip = auth_config->read<string>("network.bindip", "0.0.0.0");
			config.network_bindport = auth_config->read<unsigned short>("network.bindport", 6121);

			config.network_charip = auth_config->read<string>("network.charip", "");

			config.inter_login_ip = auth_config->read<string>("inter.login.ip", "127.0.0.1");
			config.inter_login_port = auth_config->read<unsigned short>("inter.login.port", 6900);
			config.inter_login_user = auth_config->read<string>("inter.login.user", "s1");
			config.inter_login_pass = auth_config->read<string>("inter.login.pass", "p1");

			if (config.network_charip == "")
			{
				ShowInfo("Auto-detecting my IP Address...\n");
				
				tcp::resolver resolver(*io_service);
				tcp::resolver::query query(boost::asio::ip::host_name(), "");
				tcp::resolver::iterator iter = resolver.resolve(query);
				tcp::resolver::iterator end; // End marker.
				while (iter != end)
				{
					tcp::endpoint ep = *iter++;

					if (!ep.address().is_v4())
						continue;
					
					config.network_charip = ep.address().to_string();

					break;
				}

				ShowStatus("Defaulting our IP Address to %s...\n", config.network_charip.c_str());
			}
		}
		ShowStatus("Finished reading charserver.conf.\n");

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

	TimerManager::Initialize(io_service);

	// Initialize Database System
	{
		ShowInfo("Opening connection to database...\n");

		database = database_helper::get_session(database_config);

		ShowSQL("Successfully opened database connection.\n");
	}

	connect_to_auth();

	// Initialize Network System
	{
		boost::system::error_code err;
		address_v4 bindip = address_v4::from_string(config.network_bindip, err);

		if (err)
		{
			ShowFatalError("%s\n", err.message().c_str());
			return;
		}

		server = new tcp_server(*io_service, (address)bindip, config.network_bindport);
		server->set_default_parser(CharServer::parse_from_client);

		ShowStatus("CharServer is ready and listening on %s:%d.\n", config.network_bindip.c_str(), config.network_bindport);
	}

	// Run IO service service and start pooling events
	io_service->run();
}

/*==============================================================*
* Function:	Connect to Auth-Server								*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Do the Connection between char and auth server   *
**==============================================================*/
void CharServer::connect_to_auth()
{
	auth_conn_ok = false;

	for (;;)
	{
		ShowInfo("Connecting to AuthServer on %s:%d...\n", config.inter_login_ip.c_str(), config.inter_login_port);

		boost::system::error_code ec;

		address_v4 addr = address_v4::from_string(config.inter_login_ip, ec);

		if (ec)
		{
			ShowFatalError("%s\n", ec.message().c_str());
			abort();
		}

		tcp::endpoint ep((address)addr, config.inter_login_port);

		auth_conn = tcp_connection::create(*io_service);
		auth_conn->set_parser(&CharServer::parse_from_login);
		auth_conn->socket().connect(ep, ec);

		if (ec)
		{
			ShowError("%s\n", ec.message().c_str());
			auth_conn.reset();

			{
				boost::xtime xt;

				boost::xtime_get(&xt, boost::TIME_UTC);
				xt.sec += 5;

				boost::thread::sleep(xt);
			}

			continue;
		}

		auth_conn->start();

		WFIFOHEAD(auth_conn, 76);
		WFIFOW(auth_conn, 0) = INTER_CA_LOGIN;
		strncpy((char*)WFIFOP(auth_conn, 2), config.inter_login_user.c_str(), NAME_LENGTH);
		strncpy((char*)WFIFOP(auth_conn, 26), config.inter_login_pass.c_str(), NAME_LENGTH);
		strncpy((char*)WFIFOP(auth_conn, 50), config.server_name.c_str(), 20);
		WFIFOL(auth_conn, 70) = htonl(address_v4::from_string(config.network_charip).to_ulong());
		WFIFOW(auth_conn, 74) = htons(config.network_bindport);
		auth_conn->send_buffer(76);

		break;
	}
}

/*==============================================================*
* Function:	Parse from Zone Server								*                                                     
* Author: TODO													*
* Date: TODO 													*
* Description: TODO									            *
**==============================================================*/
int CharServer::parse_from_zone(tcp_connection::pointer cl)
{
	return 0;
}

/*==============================================================*
* Function:	Parse from Login Server								*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Parse informations from auth server	            *
**==============================================================*/
int CharServer::parse_from_login(tcp_connection::pointer cl)
{
	CharSessionData *csd;

	if (cl->flags.eof)
	{
		cl->do_close();

		connect_to_auth();

		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{
		case INTER_AC_REQ_ACC_DATA_REPLY:
			if (RFIFOREST(cl) < 66)
				return 0;
			{
				int tag = RFIFOL(cl, 62);

				if (tcp_connection::session_exists(tag) && 
					(csd = (CharSessionData *)tcp_connection::get_session_by_tag(tag)->get_data()))
				{
					memcpy(csd->email, RFIFOP(cl,6), 40);
					csd->expiration_time = (time_t)RFIFOL(cl,46);
					csd->gmlevel = RFIFOB(cl,50);
					strncpy(csd->birthdate, (const char*)RFIFOP(cl,51), sizeof(csd->birthdate));

					// TODO: Check max users and min level to bypass

					send_chars(csd->account_id, csd->cl);
				}
			}
			cl->skip(66);
			break;
		case INTER_AC_AUTH_REPLY:
			if (RFIFOREST(cl) < 25)
				return 0;
			{
				int account_id = RFIFOL(cl,2);
				unsigned int login_id1 = RFIFOL(cl,6);
				unsigned int login_id2 = RFIFOL(cl,10);
				unsigned char sex = RFIFOB(cl,14);
				unsigned char result = RFIFOB(cl,15);
				int request_id = RFIFOL(cl,16);
				unsigned int version = RFIFOL(cl,20);
				unsigned char clienttype = RFIFOB(cl,24);
				cl->skip(25);

				if (tcp_connection::session_exists(request_id) && 
					(csd = (CharSessionData *)tcp_connection::get_session_by_tag(request_id)->get_data()) &&
					!csd->auth && csd->account_id == account_id && csd->login_id1 == login_id1 &&
					csd->login_id2 == login_id2 && csd->sex == sex)
				{
					tcp_connection::pointer client_fd = csd->cl;
					csd->version = version;
					csd->clienttype = clienttype;
					
					if (result == 0)
					{
						auth_ok(client_fd, csd);
					}
					else
					{
						WFIFOHEAD(client_fd,3);
						WFIFOW(client_fd,0) = HEADER_HC_REFUSE_ENTER;
						WFIFOB(client_fd,2) = 0;
						client_fd->send_buffer(3);
					}
				}
			}
			break;
		case INTER_AC_KICK:
			{
				int aid = RFIFOL(cl, 2);
				cl->skip(6);

				if (online_chars.count(aid))
				{
					if (online_chars[aid].server > -1)
					{
						// TODO: Kick from ZoneServer
					}
					else
					{
						if (!online_chars[aid].cl->flags.eof)
						{
							WFIFOHEAD(online_chars[aid].cl,3);
							WFIFOW(online_chars[aid].cl,0) = HEADER_SC_NOTIFY_BAN;
							WFIFOB(online_chars[aid].cl,2) = 2;
							online_chars[aid].cl->send_buffer(3);
							online_chars[aid].cl->set_eof();
						}
						else
							set_char_offline(aid, -1);
					}
				}
			}
			break;
		case INTER_AC_LOGIN_REPLY:
			{
				unsigned char result = RFIFOB(cl, 2);

				cl->skip(3);

				if (result == 0)
				{
					auth_conn_ok = true;

					ShowStatus("Connected to AuthServer.\n");
				}
				else
				{
					ShowError("Connectiong rejected from AuthServer.");
					cl->set_eof();
					return 0;
				}
			}
			break;
		default:
			ShowWarning("Unknown packet 0x%x sent from AuthServer, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

/*==============================================================*
* Function:	Parse from Client									*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Parse informations from client		            *
**==============================================================*/
int CharServer::parse_from_client(tcp_connection::pointer cl)
{
	CharSessionData *csd = ((CharSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		if (csd)
			delete csd;

		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{
		case HEADER_CH_ENTER:
			if(RFIFOREST(cl) < 17)
				return 0;
			{
				int account_id = RFIFOL(cl,2);
				unsigned int login_id1 = RFIFOL(cl,6);
				unsigned int login_id2 = RFIFOL(cl,10);
				char sex = RFIFOB(cl,16);
				cl->skip(17);

				if (csd)
				{
					break;
				}

				csd = new CharSessionData();
				csd->account_id = account_id;
				csd->login_id1 = login_id1;
				csd->login_id2 = login_id2;
				csd->sex = sex;
				csd->auth = false;
				csd->cl = cl;
				cl->set_data((char*)csd);

				WFIFOHEAD(cl, 4);
				WFIFOL(cl,0) = account_id;
				cl->send_buffer(4);

				if (auth_nodes.count(account_id) && 
					auth_nodes[account_id].login_id1  == login_id1 &&
					auth_nodes[account_id].login_id2  == login_id2)
				{
					auth_nodes.erase(account_id);
					auth_ok(cl, csd);
				}
				else
				{
					if (auth_conn_ok)
					{
						WFIFOHEAD(auth_conn,23);
						WFIFOW(auth_conn,0) = INTER_CA_AUTH;
						WFIFOL(auth_conn,2) = csd->account_id;
						WFIFOL(auth_conn,6) = csd->login_id1;
						WFIFOL(auth_conn,10) = csd->login_id2;
						WFIFOB(auth_conn,14) = csd->sex;
						WFIFOL(auth_conn,15) = htonl(cl->socket().remote_endpoint().address().to_v4().to_ulong());
						WFIFOL(auth_conn,19) = cl->tag();
						auth_conn->send_buffer(23);
					}
					else
					{
						WFIFOHEAD(cl,3);
						WFIFOW(cl,0) = HEADER_HC_REFUSE_ENTER;
						WFIFOB(cl,2) = 0;
						cl->send_buffer(3);
					}
				}
			}
			break;
		case HEADER_PING:
			if (RFIFOREST(cl) < 6)
				return 0;
			cl->skip(6);
			break;
		default:
			ShowWarning("Unknown packet 0x%04x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

/*==============================================================*
* Function:	Connection Success into Char Server					*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description:										            *
**==============================================================*/
void CharServer::auth_ok(tcp_connection::pointer cl, CharSessionData *csd)
{
	if (online_chars.count(csd->account_id))
	{
		if (online_chars[csd->account_id].server > -1)
		{
			// TODO: Kick form ZoneServer

			if (online_chars[csd->account_id].disconnect_timer)
				TimerManager::FreeTimer(online_chars[csd->account_id].disconnect_timer);

			set_char_offline(csd->account_id, -1);

			WFIFOHEAD(cl,3);	
			WFIFOW(cl,0) = HEADER_SC_NOTIFY_BAN;
			WFIFOB(cl,2) = 8;
			cl->send_buffer(3);

			return;
		}
		
		if (online_chars[csd->account_id].cl->tag() != cl->tag())
		{
			WFIFOHEAD(cl,3);	
			WFIFOW(cl,0) = HEADER_SC_NOTIFY_BAN;
			WFIFOB(cl,2) = 8;
			cl->send_buffer(3);

			return;
		}

		online_chars[csd->account_id].cl = cl;
	}

	if (auth_conn_ok)
	{
		WFIFOHEAD(auth_conn,10);
		WFIFOW(auth_conn,0) = INTER_CA_REQ_ACC_DATA;
		WFIFOL(auth_conn,2) = csd->account_id;
		WFIFOL(auth_conn,6) = cl->tag();
		auth_conn->send_buffer(10);
	}

	set_charsel(csd->account_id, cl);
}

/*==============================================================*
* Function:	Disconnect Char										*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Disconnect an char from charserver		        *
**==============================================================*/
void CharServer::disconnect_char(int timer, int accid)
{
	set_char_offline(accid, -1);
}


/*==============================================================*
* Function:	Character Selection									*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description:											        *
**==============================================================*/
void CharServer::set_charsel(int account_id, tcp_connection::pointer cl)
{
	// TODO: Decrement ZoneServer user on

	online_chars[account_id].char_id = -1;
	online_chars[account_id].server = -1;
	online_chars[account_id].cl = cl;

	if (online_chars[account_id].disconnect_timer) 
		TimerManager::FreeTimer(online_chars[account_id].disconnect_timer);

	if (auth_conn_ok)
	{
		WFIFOHEAD(auth_conn,6);
		WFIFOW(auth_conn,0) = INTER_CA_SET_ACC_ON;
		WFIFOL(auth_conn,2) = account_id;
		auth_conn->send_buffer(6);
	}
}

/*==============================================================*
* Function:	Set a character to offline							*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description:											        *
**==============================================================*/
void CharServer::set_char_offline(int account_id, char char_id)
{
	if (online_chars.count(account_id))
	{
		if (online_chars[account_id].server > -1)
		{
			// TODO: Decrement ZoneServer users online
		}

		if (online_chars[account_id].disconnect_timer)
			TimerManager::FreeTimer(online_chars[account_id].disconnect_timer);

		if (online_chars[account_id].char_id == char_id)
		{
			online_chars[account_id].char_id = -1;
			online_chars[account_id].server = -1;
		}
	}

	if (auth_conn_ok && (char_id == -1 || !online_chars.count(account_id) || online_chars[account_id].cl->flags.eof))
	{
		WFIFOHEAD(auth_conn,6);
		WFIFOW(auth_conn,0) = INTER_CA_SET_ACC_OFF;
		WFIFOL(auth_conn,2) = account_id;
		auth_conn->send_buffer(6);
	}
}

#define MAX_CHAR_BUF 140

/*==============================================================*
* Function:	Send Characters										*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Send to the client character infos		        *
**==============================================================*/
void CharServer::send_chars(int account_id, tcp_connection::pointer cl)
{
	int i, j, found_num, offset = 0;
#if PACKETVER >= 20100413
	offset += 3;
#endif

	found_num = 0;//char_find_characters(sd);

	j = 24 + offset; // offset
	WFIFOHEAD(cl,j + found_num*MAX_CHAR_BUF);
	WFIFOW(cl,0) = HEADER_HC_ACCEPT_ENTER;
#if PACKETVER >= 20100413
	WFIFOB(cl,4) = MAX_CHARS_SLOTS;
	WFIFOB(cl,5) = MAX_CHARS;
	WFIFOB(cl,6) = MAX_CHARS;
#endif
	memset(WFIFOP(cl,4 + offset), 0, 20);
	//for(i = 0; i < found_num; i++)
	//	j += mmo_char_tobuf(WFIFOP(cl,j), &char_dat[sd->found_char[i]].status);
	WFIFOW(cl,2) = j;
	cl->send_buffer(j);

	return;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	core_display_title();

	CharServer::run();

	getchar();
	return 0;
}
