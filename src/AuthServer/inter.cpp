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
*                  Authentication Server				   *
* ======================================================== */

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

/*==============================================================*
* Function:	Send all WoS										*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description:									                *
**==============================================================*/
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

/*==============================================================*
* Function:	Parse from Char										*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Parse informations from char-server              *
**==============================================================*/
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
				if (pair.second.char_server == asd->account_id)
				{
					if (pair.second.enter_charserver_timeout)
						TimerManager::FreeTimer(pair.second.enter_charserver_timeout);

					// FIXME: Do this inside this loop(actually it breaks the iterator)
					pair.second.enter_charserver_timeout = TimerManager::CreateStartTimer(500, 
						false, boost::bind(&AuthServer::select_charserver_timeout, 
						_1, pair.first));
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
		case INTER_CA_SET_ACC_OFF:
			if(RFIFOREST(cl) < 6)
				return 0;
			{
				shutdown_account(RFIFOL(cl,2));
				cl->skip(6);
			}
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

				if (accounts->load_account(account_id, acc))
				{
					expiration_time = acc.expiration_time;
					strncpy(email, acc.email.c_str(), 40);
					gmlevel = acc.level;
					strncpy(birthdate, acc.birthdate.c_str(), 11);
				}

				WFIFOHEAD(cl,62);
				WFIFOW(cl,0) = INTER_AC_REQ_ACC_DATA_REPLY;
				WFIFOL(cl,2) = rid;
				strncpy((char*)WFIFOP(cl,6), email, 40);
				WFIFOL(cl,46) = (unsigned int)expiration_time;
				WFIFOB(cl,50) = gmlevel;
				strncpy((char*)WFIFOP(cl,51), birthdate, 11);
				cl->send_buffer(62);
			}
			break;
		case INTER_CA_AUTH:
			if(RFIFOREST(cl) < 19)
				return 0;
			{
				int account_id = RFIFOL(cl,2);
				unsigned int login_id1 = RFIFOL(cl,6);
				unsigned int login_id2 = RFIFOL(cl,10);
				unsigned char sex = RFIFOB(cl,14);
				int request_id = RFIFOL(cl,15);
				cl->skip(19);

				// Find the temporary authentication node
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
					cl->send_buffer(20);

					if (online_accounts[account_id].enter_charserver_timeout)
						TimerManager::FreeTimer(online_accounts[account_id].enter_charserver_timeout);
					online_accounts[account_id].char_server = asd->account_id;

					auth_nodes.erase(account_id);
				}
				else
				{
					// Invalid node, timeout or hack attempt?
					WFIFOHEAD(cl,25);
					WFIFOW(cl,0) = INTER_AC_AUTH_REPLY;
					WFIFOL(cl,2) = account_id;
					WFIFOL(cl,6) = login_id1;
					WFIFOL(cl,10) = login_id2;
					WFIFOB(cl,14) = sex;
					WFIFOB(cl,15) = 1;
					WFIFOL(cl,16) = request_id;
					cl->send_buffer(20);
				}
			}
			break;

		default:
			ShowWarning("Unknown packet 0x%04x sent from CharServer '%s', closing connection.\n", cmd, servers[asd->account_id].name);
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}
