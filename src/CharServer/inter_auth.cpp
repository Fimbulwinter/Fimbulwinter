/*==================================================================*
*     ___ _           _           _          _       _				*
*    / __(_)_ __ ___ | |__  _   _| |_      _(_)_ __ | |_ ___ _ __	*
*   / _\ | | '_ ` _ \| '_ \| | | | \ \ /\ / / | '_ \| __/ _ \ '__|	*
*  / /   | | | | | | | |_) | |_| | |\ V  V /| | | | | ||  __/ |		*
*  \/    |_|_| |_| |_|_.__/ \__,_|_| \_/\_/ |_|_| |_|\__\___|_|		*
*																	*
* ------------------------------------------------------------------*
*							 Emulator   			                *
* ------------------------------------------------------------------*
*                     Licenced under GNU GPL v3                     *
* ----------------------------------------------------------------- *
*                   Character Server to Auth Server        	        *
* ==================================================================*/
#pragma once

#include "CharServer.hpp"

#include  "../Common/show_message.hpp"
#include  "../Common/database_helper.h"
#include  "../Common/ragnarok.hpp"
#include  "../Common/packets.hpp"
#include  "../Common/timers.hpp"
#include  "../Common/strfuncs.hpp"
#include  "../Common/core.hpp"


#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <iostream>

/*! 
 *  \brief     Connect to Auth Server
 *  \details   Do the connection between char and auth server
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
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


/*! 
 *  \brief     Parse from Login
 *  \details   Parse informations from auth server
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
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
			if (RFIFOREST(cl) < 62)
				return 0;
			{
				int tag = RFIFOL(cl, 2);

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
			cl->skip(62);
			break;
		case INTER_AC_AUTH_REPLY:
			if (RFIFOREST(cl) < 20)
				return 0;
			{
				int account_id = RFIFOL(cl,2);
				unsigned int login_id1 = RFIFOL(cl,6);
				unsigned int login_id2 = RFIFOL(cl,10);
				unsigned char sex = RFIFOB(cl,14);
				unsigned char result = RFIFOB(cl,15);
				int request_id = RFIFOL(cl,16);
				cl->skip(20);

				if (tcp_connection::session_exists(request_id) && 
					(csd = (CharSessionData *)tcp_connection::get_session_by_tag(request_id)->get_data()) &&
					!csd->auth && csd->account_id == account_id && csd->login_id1 == login_id1 &&
					csd->login_id2 == login_id2 && csd->sex == sex)
				{
					tcp_connection::pointer client_cl = csd->cl;

					if (result == 0)
					{
						auth_ok(client_cl, csd);
					}
					else
					{
						WFIFOPACKET(client_cl,packet,HC_REFUSE_ENTER);
						packet->header = HEADER_HC_REFUSE_ENTER;
						packet->error_code = 0;
						client_cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_ENTER));
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
							WFIFOPACKET(online_chars[aid].cl,packet,SC_NOTIFY_BAN);
							packet->header = HEADER_SC_NOTIFY_BAN;
							packet->error_code = 2;
							online_chars[aid].cl->send_buffer(sizeof(struct PACKET_SC_NOTIFY_BAN));
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

///! \brief Request Acc Register
void CharServer::request_accreg2( int account_id, int char_id )
{
	throw std::exception("The method or operation is not implemented.");
}
