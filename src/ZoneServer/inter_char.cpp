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

#include "ZoneServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <boost/thread.hpp>
#include <ragnarok.hpp>
#include <packets.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>
#include <boost/foreach.hpp>
#include <strfuncs.hpp>
#include "MapManager.hpp"
#include "Player.hpp"

/*==============================================================*
* Function:	Connect to Char-Server								*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Do the Connection between char and auth server   *
**==============================================================*/
void ZoneServer::connect_to_char()
{
	char_conn_ok = false;

	for (;;)
	{
		ShowInfo("Connecting to CharServer on %s:%d...\n", config.inter_char_ip.c_str(), config.inter_char_port);

		boost::system::error_code ec;

		address_v4 addr = address_v4::from_string(config.inter_char_ip, ec);

		if (ec)
		{
			ShowFatalError("%s\n", ec.message().c_str());
			abort();
		}

		tcp::endpoint ep((address)addr, config.inter_char_port);

		char_conn = tcp_connection::create(*io_service);
		char_conn->set_parser(&ZoneServer::parse_from_char);
		char_conn->socket().connect(ep, ec);

		if (ec)
		{
			ShowError("%s\n", ec.message().c_str());
			char_conn.reset();

			{
				boost::xtime xt;

				boost::xtime_get(&xt, boost::TIME_UTC);
				xt.sec += 5;

				boost::thread::sleep(xt);
			}

			continue;
		}

		char_conn->start();

		WFIFOHEAD(char_conn, 60);
		WFIFOW(char_conn, 0) = INTER_ZC_LOGIN;
		memcpy((char*)WFIFOP(char_conn, 2), config.inter_char_user.c_str(), NAME_LENGTH);
		memcpy((char*)WFIFOP(char_conn, 26), config.inter_char_pass.c_str(), NAME_LENGTH);
		WFIFOL(char_conn,50) = 0;
		WFIFOL(char_conn,54) = htonl((address_v4::from_string(config.network_zoneip)).to_ulong());
		WFIFOW(char_conn,58) = htons(config.network_bindport);
		char_conn->send_buffer(60);

		break;
	}
}

/*==============================================================*
* Function:	Parse from Char Server								*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Parse informations from auth server	            *
**==============================================================*/
int ZoneServer::parse_from_char(tcp_connection::pointer cl)
{
	if (cl->flags.eof)
	{
		cl->do_close();

		connect_to_char();

		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{

		case INTER_ZC_REQ_REGS_REPLY:
			if (RFIFOREST(cl) < 4 || RFIFOREST(cl) < RFIFOW(cl, 2))
				return 0;
			{
				int j, p, len, max, flag;
				struct ZoneSessionData *sd;
				struct GlobalReg *reg;
				int *qty;
				int account_id = RFIFOL(cl,4), char_id = RFIFOL(cl,8);

				sd = BlockManager::get_session(account_id);
				if (sd && RFIFOB(cl,12) == 3 && sd->status.char_id != char_id)
					sd = NULL;
				
				if (!sd) 
					break;

				flag = (sd->save_reg.global_num == -1 || sd->save_reg.account_num == -1 || sd->save_reg.account2_num == -1);

				switch (RFIFOB(cl,12)) 
				{
				case 3: //Character Registry
					reg = sd->save_reg.global;
					qty = &sd->save_reg.global_num;
					max = GLOBAL_REG_NUM;
					break;
				case 2: //Account Registry
					reg = sd->save_reg.account;
					qty = &sd->save_reg.account_num;
					max = ACCOUNT_REG_NUM;
					break;
				case 1: //Account2 Registry
					reg = sd->save_reg.account2;
					qty = &sd->save_reg.account2_num;
					max = ACCOUNT_REG2_NUM;
					break;
				default:
					ShowError("intif_parse_Registers: Unrecognized type %d\n", RFIFOB(cl,12));
					return 0;
				}

				for(j = 0, p = 13; j < max && p < RFIFOW(cl,2); j++)
				{
					sscanf((char *)RFIFOP(cl, p), "%31c%n", reg[j].str, &len);
					reg[j].str[len] = '\0';
					p += len + 1;

					sscanf((char *)RFIFOP(cl, p), "%255c%n", reg[j].value, &len);
					reg[j].value[len] = '\0';
					p += len + 1;
				}
				*qty = j;

				if (flag && sd->save_reg.global_num > -1 && sd->save_reg.account_num > -1 && sd->save_reg.account2_num > -1)
					PC::reg_received(sd);
			}
			break;

		case INTER_CZ_LOGIN_REPLY:
			if (RFIFOREST(cl) < 3)
				return 0;
			{
				char status = RFIFOB(cl, 2);
				cl->skip(3);

				if (status == 0)
				{
					char_conn_ok = true;

					ShowStatus("Connected to CharServer.\n");
				}
				else
				{
					ShowError("Connection rejected from CharServer.");
					cl->set_eof();
					return 0;
				}

				send_maps();
			}
			break;

		case INTER_CZ_AUTH_FAIL:
			if (RFIFOREST(cl) < 6)
				return 0;
			{
				int acc = RFIFOL(cl, 2);
				cl->skip(6);

				if (auth_nodes.count(acc))
				{
					auth_fail(auth_nodes[acc].cl, 0);

					auth_nodes.erase(acc);
				}
			}
			break;

		case INTER_CZ_AUTH_OK:
			if (RFIFOREST(cl) < 24 + sizeof(struct CharData))
				return 0;
			{
				int account_id;
				unsigned int login_id1;
				unsigned int login_id2;
				time_t expiration_time;
				int gmlevel;
				struct CharData *status;
				int char_id;
				ZoneSessionData *sd;

				//Check if both servers agree on the struct's size
				if(RFIFOW(cl,2) - 24 != sizeof(struct CharData))
				{
					ShowError("INTER_CZ_AUTH_FAIL: Data size mismatch! %d != %d\n", RFIFOW(cl,2) - 24, sizeof(struct CharData));
					return 0;
				}

				account_id = RFIFOL(cl,4);
				login_id1 = RFIFOL(cl,8);
				login_id2 = RFIFOL(cl,12);
				expiration_time = (time_t)RFIFOL(cl,16);
				gmlevel = RFIFOL(cl,20);
				status = (struct CharData *)RFIFOP(cl,24);

				char_id = status->char_id;

				if ((sd = BlockManager::get_session(account_id)) != NULL)
					return 0;
	
				if (!auth_nodes.count(account_id))
					return 0;

				if (auth_nodes[account_id].auth_state != ST_LOGIN)
					return 0;

				if (auth_nodes[account_id].sd == NULL)
				{
					return 0;
				}

				sd = auth_nodes[account_id].sd;
				if(auth_nodes[account_id].char_dat == NULL &&
					auth_nodes[account_id].account_id == account_id &&
					auth_nodes[account_id].char_id == char_id &&
					auth_nodes[account_id].login_id1 == login_id1 )
				{ 
					//Auth Ok
					if (PC::auth_ok(sd, login_id2, expiration_time, gmlevel, status))
						return 0;
				} 
				else 
				{ 
					//Auth Failed
					PC::auth_fail(sd);
				}

				set_char_offline(account_id, char_id);
				auth_nodes.erase(account_id);
			}
			cl->skip(24 + sizeof(struct CharData));

			break;

		default:
			ShowWarning("Unknown packet 0x%x sent from CharServer, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

void ZoneServer::send_maps()
{
	if (char_conn_ok)
	{
		WFIFOHEAD(char_conn, 4 + sizeof(int) * MapManager::maps.size());
		WFIFOW(char_conn, 0) = INTER_ZC_MAPS;
		WFIFOW(char_conn, 2) = 4 + sizeof(int) * MapManager::maps.size();

		int i = 0;
		BOOST_FOREACH(MapData &map, MapManager::maps)
		{
			WFIFOL(char_conn, 4 + (i * sizeof(int))) = map.m;

			i++;
		}

		char_conn->send_buffer(4 + sizeof(int) * MapManager::maps.size());
	}
}

void ZoneServer::inter_confirm_auth(ZoneSessionData *sd)
{
	if (char_conn_ok)
	{
		WFIFOHEAD(char_conn, 15);
		WFIFOW(char_conn, 0) = INTER_ZC_AUTH;
		WFIFOL(char_conn, 2) = sd->status.account_id;
		WFIFOL(char_conn, 6) = sd->status.char_id;
		WFIFOL(char_conn, 10) = sd->login_id1;
		WFIFOB(char_conn, 14) = sd->status.sex;
		char_conn->send_buffer(15);

		create_auth_entry(sd, ST_LOGIN);
	}
}

void ZoneServer::request_registry( ZoneSessionData * sd, int flag )
{
	if (!sd)
		return;

	sd->save_reg.account2_num = -1;
	sd->save_reg.account_num = -1;
	sd->save_reg.global_num = -1;

	WFIFOHEAD(char_conn,6);
	WFIFOW(char_conn,0) = INTER_ZC_REQ_REGS;
	WFIFOL(char_conn,2) = sd->status.account_id;
	WFIFOL(char_conn,6) = sd->status.char_id;
	WFIFOB(char_conn,10) = (flag & 1 ? 1 : 0); // Request Account Reg 2
	WFIFOB(char_conn,11) = (flag & 2 ? 1 : 0); // Request Account Reg
	WFIFOB(char_conn,12) = (flag & 4 ? 1 : 0); // Request Char Reg
	char_conn->send_buffer(13);
}

