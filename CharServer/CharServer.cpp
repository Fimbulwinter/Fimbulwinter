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
#include <strfuncs.hpp>

#define MAX_CHAR_BUF 140

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
CharDB *CharServer::chars;

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

		try
		{
			database = database_helper::get_session(database_config);
		}
		catch (soci::soci_error err)
		{
			ShowFatalError("Error opening database connection: %s\n", err.what());
			return;
		}

		chars = new CharDB(database);

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

					csd->auth = true;
					send_chars(csd->cl, csd);
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
					tcp_connection::pointer client_cl = csd->cl;
					csd->version = version;
					csd->clienttype = clienttype;
					
					if (result == 0)
					{
						auth_ok(client_cl, csd);
					}
					else
					{
						WFIFOHEAD(client_cl,3);
						WFIFOW(client_cl,0) = HEADER_HC_REFUSE_ENTER;
						WFIFOB(client_cl,2) = 0;
						client_cl->send_buffer(3);
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

		#define FIFOSD_CHECK(rest) { if(RFIFOREST(cl) < rest) return 0; if (csd==NULL || !csd->auth) { cl->skip(rest); return 0; } }

		switch (cmd)
		{
		case HEADER_CH_REQUEST_DEL_TIMER:
			FIFOSD_CHECK(6);
			delete2_req(cl, csd);
			cl->skip(6);
			break;

		case HEADER_CH_ACCEPT_DEL_REQ:
			FIFOSD_CHECK(12);
			delete2_accept(cl, csd);
			cl->skip(6);
			break;

		case HEADER_CH_CANCEL_DEL_REQ:
			FIFOSD_CHECK(6);
			delete2_cancel(cl, csd);
			cl->skip(6);
			break;
		case HEADER_CH_DELETE_CHAR:
		case HEADER_CH_DELETE_CHAR2:
			if (cmd == 0x68) FIFOSD_CHECK(46);
			if (cmd == 0x1fb) FIFOSD_CHECK(56);
			FIFOSD_CHECK(37);
			{
				int cid = RFIFOL(cl,2);
				char email[40];
				memcpy(email, RFIFOP(cl,6), 40);

				cl->skip((cmd == 0x68) ? 46 : 56);

				if (_strcmpi(email, csd->email) != 0 && (strcmp("a@a.com", csd->email) || (strcmp("a@a.com", email) && strcmp("", email))))
				{
					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = HEADER_HC_REFUSE_DELETECHAR;
					WFIFOB(cl,2) = 0;
					cl->send_buffer(3);
					break;
				}

				bool found = false;
				int i, ch;
				for (i = 0; i < MAX_CHARS; i++)
				{
					if (csd->found_char[i] == cid)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = HEADER_HC_REFUSE_DELETECHAR;
					WFIFOB(cl,2) = 0;
					cl->send_buffer(3);
					break;
				}
				else
				{
					for(ch = i; ch < MAX_CHARS - 1; ch++)
						csd->found_char[ch] = csd->found_char[ch+1];

					csd->found_char[MAX_CHARS - 1] = -1;

					if (!chars->delete_char(cid))
					{
						WFIFOHEAD(cl,3);
						WFIFOW(cl,0) = HEADER_HC_REFUSE_DELETECHAR;
						WFIFOB(cl,2) = 0;
						cl->send_buffer(3);
						break;
					}

					WFIFOHEAD(cl,2);
					WFIFOW(cl,0) = HEADER_HC_ACCEPT_DELETECHAR;
					cl->send_buffer(2);
				}
			}
			break;
		case HEADER_CH_MAKE_CHAR:
			FIFOSD_CHECK(37);
			{
				// TODO: Check create char disabled
				int i = create_char(csd, (char*)RFIFOP(cl,2),RFIFOB(cl,26),RFIFOB(cl,27),RFIFOB(cl,28),RFIFOB(cl,29),RFIFOB(cl,30),RFIFOB(cl,31),RFIFOB(cl,32),RFIFOW(cl,33),RFIFOW(cl,35));

				//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
				if (i < 0)
				{
					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = HEADER_HC_REFUSE_MAKECHAR;
					switch (i) {
					case -1: WFIFOB(cl,2) = 0x00; break;
					case -2: WFIFOB(cl,2) = 0xFF; break;
					case -3: WFIFOB(cl,2) = 0x01; break;
					}
					cl->send_buffer(3);
				}
				else
				{
					int len;

					// retrieve data
					CharData char_dat;
					chars->load_char(i, char_dat, false); //Only the short data is needed.

					// send to player
					WFIFOHEAD(cl,2+MAX_CHAR_BUF);
					WFIFOW(cl,0) = HEADER_HC_ACCEPT_MAKECHAR;
					len = 2 + char_to_buf(WFIFOP(cl,2), &char_dat);
					cl->send_buffer(len);

					// add new entry to the chars list
					for (int n = 0; n < MAX_CHARS; n++)
					{
						if(csd->found_char[n] == -1)
							csd->found_char[n] = i; // the char_id of the new char
					}
				}
				cl->skip(37);
			}
			break;
		case HDADER_CH_ENTER_CHECKBOT:
			WFIFOHEAD(cl,5);
			WFIFOW(cl,0) = 0x7e9;
			WFIFOW(cl,2) = 5;
			WFIFOB(cl,4) = 1;
			cl->send_buffer(5);
			cl->skip(8);
			break;
		case HEADER_CH_CHECKBOT:
			WFIFOHEAD(cl,5);
			WFIFOW(cl,0) = 0x7e9;
			WFIFOW(cl,2) = 5;
			WFIFOB(cl,4) = 1;
			cl->send_buffer(5);
			cl->skip(32);
			break;
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
/*==============================================================*
* Function:	Send Characters										*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Send to the client character infos		        *
**==============================================================*/
void CharServer::send_chars(tcp_connection::pointer cl, CharSessionData *csd)
{
	int j, offset = 0;
#if PACKETVER >= 20100413
	offset += 3;
#endif

	j = 24 + offset; // offset
	WFIFOHEAD(cl,j + MAX_CHARS*MAX_CHAR_BUF);
	WFIFOW(cl,0) = HEADER_HC_ACCEPT_ENTER;
#if PACKETVER >= 20100413
	WFIFOB(cl,4) = MAX_CHARS_SLOTS;
	WFIFOB(cl,5) = MAX_CHARS;
	WFIFOB(cl,6) = MAX_CHARS;
#endif
	memset(WFIFOP(cl,4 + offset), 0, 20);
	j += chars->load_chars_to_buf(csd->account_id, (char*)WFIFOP(cl,j), csd);
	WFIFOW(cl,2) = j;
	cl->send_buffer(j);

	return;
}

int CharServer::char_to_buf(unsigned char *buffer, CharData *p)
{
	unsigned short offset = 0;
	unsigned char *buf;

	if( buffer == NULL || p == NULL )
		return 0;

	buf = WBUFP(buffer,0);
	WBUFL(buf,0) = p->char_id;
	WBUFL(buf,4) = min<unsigned int>(p->base_exp, INT32_MAX);
	WBUFL(buf,8) = p->zeny;
	WBUFL(buf,12) = min<unsigned int>(p->job_exp, INT32_MAX);
	WBUFL(buf,16) = p->job_level;
	WBUFL(buf,20) = 0; // probably opt1
	WBUFL(buf,24) = 0; // probably opt2
	WBUFL(buf,28) = p->option;
	WBUFL(buf,32) = p->karma;
	WBUFL(buf,36) = p->manner;
	WBUFW(buf,40) = min<unsigned short>(p->status_point, INT16_MAX);
#if PACKETVER > 20081217
	WBUFL(buf,42) = p->hp;
	WBUFL(buf,46) = p->max_hp;
	offset+=4;
	buf = WBUFP(buffer,offset);
#else
	WBUFW(buf,42) = min<unsigned short>(p->hp, INT16_MAX);
	WBUFW(buf,44) = min<unsigned short>(p->max_hp, INT16_MAX);
#endif
	WBUFW(buf,46) = min<unsigned short>(p->sp, INT16_MAX);
	WBUFW(buf,48) = min<unsigned short>(p->max_sp, INT16_MAX);
	WBUFW(buf,50) = DEFAULT_WALK_SPEED; // p->speed;
	WBUFW(buf,52) = p->class_;
	WBUFW(buf,54) = p->hair;
	WBUFW(buf,56) = p->option&0x7E80020 ? 0 : p->weapon; //When the weapon is sent and your option is riding, the client crashes on login!?
	WBUFW(buf,58) = p->base_level;
	WBUFW(buf,60) = min<unsigned short>(p->skill_point, INT16_MAX);
	WBUFW(buf,62) = p->head_bottom;
	WBUFW(buf,64) = p->shield;
	WBUFW(buf,66) = p->head_top;
	WBUFW(buf,68) = p->head_mid;
	WBUFW(buf,70) = p->hair_color;
	WBUFW(buf,72) = p->clothes_color;
	memcpy(WBUFP(buf,74), p->name.c_str(), NAME_LENGTH);
	WBUFB(buf,98) = (unsigned char)min<unsigned short>(p->str, UINT8_MAX);
	WBUFB(buf,99) = (unsigned char)min<unsigned short>(p->agi, UINT8_MAX);
	WBUFB(buf,100) = (unsigned char)min<unsigned short>(p->vit, UINT8_MAX);
	WBUFB(buf,101) = (unsigned char)min<unsigned short>(p->int_, UINT8_MAX);
	WBUFB(buf,102) = (unsigned char)min<unsigned short>(p->dex, UINT8_MAX);
	WBUFB(buf,103) = (unsigned char)min<unsigned short>(p->luk, UINT8_MAX);
	WBUFW(buf,104) = p->slot;
#if PACKETVER >= 20061023
	WBUFW(buf,106) = ( p->rename > 0 ) ? 0 : 1;
	offset += 2;
#endif
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
	//mapindex_getmapname_ext(mapindex_id2name(p->last_point.map), (char*)WBUFP(buf,108));
	strncpy((char*)WBUFP(buf,108), "prontera.gat", MAP_NAME_LENGTH_EXT);
	offset += MAP_NAME_LENGTH_EXT;
#endif
#if PACKETVER >= 20100803
	WBUFL(buf,124) = TOL(p->delete_date);
	offset += 4;
#endif
#if PACKETVER >= 20110111
	WBUFL(buf,128) = p->robe;
	offset += 4;
#endif
#if PACKETVER >= 20110928
	WBUFL(buf,132) = 0;  // change slot feature (0 = disabled, otherwise enabled)
	offset += 4;
#endif
	return 106+offset;
}

int CharServer::create_char(CharSessionData *csd, char* name, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style)
{
	int char_id, flag;

	flag = check_char_name(name);
	if( flag < 0 )
		return flag;

	if((slot >= MAX_CHARS) // slots
		|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
		|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
		|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
		return -2; // invalid input

	// TODO: Check max chars per account

	{
		statement s = (database->prepare << "SELECT 1 FROM `char` WHERE `account_id`=:acc AND `char_num`=:slot", use(csd->account_id), use(slot));
		
		s.execute(false);

		if (s.get_affected_rows())
			return -2;
	}

	CharData c;
	memset(&c, 0, sizeof(CharData));
	c.account_id = csd->account_id;
	c.str = str;
	c.agi = agi;
	c.vit = vit;
	c.int_ = int_;
	c.dex = dex;
	c.luk = luk;
	c.name = string(name);
	c.slot = slot;
	c.hair_color = hair_color;
	c.hair = hair_style;
	char_id = chars->save(c, true);

	// TODO: Add initial items

	return char_id;
}

int CharServer::check_char_name(char *name)
{
	//int i;

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name

	// TODO: Check for reserved names/characters

	// check name (already in use?)
	{
		statement s = (database->prepare << "SELECT `char_id` FROM `char` WHERE `name`=:n", use(string(name)));
	
		s.execute(false);

		if (s.get_affected_rows())
			return -1;
	}

	return 0;
}

bool CharServer::check_email(char *email)
{
	char ch;
	char* last_arobas;
	size_t len = strlen(email);

	// Server Limits
	if (len < 3 || len > 39)
		return 0;

	// Part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == NULL || email[len-1] == '@')
		return 0;

	if (email[len-1] == '.')
		return 0;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != NULL || strstr(last_arobas, "..") != NULL)
		return 0;

	for(ch = 1; ch < 32; ch++)
		if (strchr(last_arobas, ch) != NULL)
			return 0;

	if (strchr(last_arobas, ' ') != NULL || strchr(last_arobas, ';') != NULL)
		return 0;

	// All Correct
	return 1;
}

void CharServer::delete2_req(tcp_connection::pointer cl, CharSessionData *csd)
{
	int char_id, i;
	time_t delete_date;

	char_id = RFIFOL(cl, 2);

	bool found = false;
	//int ch;
	for (i = 0; i < MAX_CHARS; i++)
	{
		if (csd->found_char[i] == char_id)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		delete2_ack(cl, char_id, 3, 0);
		return;
	}
	else
	{
		// TODO: Implement char deletion delay
		delete_date = time(NULL) + 60;

		*database << "UPDATE `char` SET `delete_date`=:t WHERE `char_id`=:c", use(delete_date), use(char_id);

		delete2_ack(cl, char_id, 1, delete_date);
	}
}

void CharServer::delete2_accept( tcp_connection::pointer cl, CharSessionData * csd )
{
	char birthdate[8+1];
	int char_id, i, k;
	//unsigned int base_level;
	//char* data;
	time_t delete_date;

	char_id = RFIFOL(cl,2);

	ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", csd->account_id, char_id);

	// construct "YY-MM-DD"
	birthdate[0] = RFIFOB(cl,6);
	birthdate[1] = RFIFOB(cl,7);
	birthdate[2] = '-';
	birthdate[3] = RFIFOB(cl,8);
	birthdate[4] = RFIFOB(cl,9);
	birthdate[5] = '-';
	birthdate[6] = RFIFOB(cl,10);
	birthdate[7] = RFIFOB(cl,11);
	birthdate[8] = 0;

	bool found = false;
	for (i = 0; i < MAX_CHARS; i++)
	{
		if (csd->found_char[i] == char_id)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		delete2_accept_ack(cl, char_id, 3);
		return;
	}

	*database << "SELECT `delete_date` FROM `char` WHERE `char_id` = :c", use(char_id), into(delete_date);

	if( !delete_date || delete_date>time(NULL) )
	{// not queued or delay not yet passed
		delete2_accept_ack(cl, char_id, 4);
		return;
	}

	if(strcmp(csd->birthdate + 2, birthdate))  // +2 to cut off the century
	{
		delete2_accept_ack(cl, char_id, 5);
		return;
	}

	if(!chars->delete_char(char_id))
	{
		delete2_accept_ack(cl, char_id, 3);
		return;
	}

	for(k = i; k < MAX_CHARS - 1; k++)
		csd->found_char[k] = csd->found_char[k+1];

	csd->found_char[MAX_CHARS - 1] = -1;

	delete2_accept_ack(cl, char_id, 1);
}

void CharServer::delete2_cancel( tcp_connection::pointer cl, CharSessionData * csd )
{
	int char_id, i;

	char_id = RFIFOL(cl,2);

	bool found = false;
	for (i = 0; i < MAX_CHARS; i++)
	{
		if (csd->found_char[i] == char_id)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		delete2_accept_ack(cl, char_id, 3);
		return;
	}

	// there is no need to check, whether or not the character was
	// queued for deletion, as the client prints an error message by
	// itself, if it was not the case (@see char_delete2_cancel_ack)
	*database << "UPDATE `char` SET `delete_date` = 0 WHERE `char_id`=:c", use(char_id);

	delete2_cancel_ack(cl, char_id, 1);
}

void CharServer::delete2_ack( tcp_connection::pointer cl, int char_id, int result, time_t deltime )
{
	WFIFOHEAD(cl,14);
	WFIFOW(cl,0) = HEADER_HC_DEL_REQUEST_ACK;
	WFIFOL(cl,2) = char_id;
	WFIFOL(cl,6) = result;
	WFIFOL(cl,10) = TOL(deltime);
	cl->send_buffer(14);
}

void CharServer::delete2_accept_ack( tcp_connection::pointer cl, int char_id, int result )
{
	WFIFOHEAD(cl,10);
	WFIFOW(cl,0) = HEADER_HC_DEL_ACCEPT_ACK;
	WFIFOL(cl,2) = char_id;
	WFIFOL(cl,6) = result;
	cl->send_buffer(10);
}

void CharServer::delete2_cancel_ack( tcp_connection::pointer cl, int char_id, int result )
{
	WFIFOHEAD(cl,10);
	WFIFOW(cl,0) = HEADER_HC_DEL_CANCEL_ACK;
	WFIFOL(cl,2) = char_id;
	WFIFOL(cl,6) = result;
	cl->send_buffer(10);
}

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	core_display_title();

	CharServer::run();

	getchar();
	return 0;
}
