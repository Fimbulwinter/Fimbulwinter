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
		WFIFOHEAD(char_conn, 4 + sizeof(int) * my_maps.size());
		WFIFOW(char_conn, 0) = INTER_ZC_MAPS;
		WFIFOW(char_conn, 2) = 4 + sizeof(int) * my_maps.size();

		int i = 0;
		BOOST_FOREACH(int m, my_maps)
		{
			WFIFOL(char_conn, 4 + (i * sizeof(int))) = m;

			i++;
		}

		char_conn->send_buffer(4 + sizeof(int) * my_maps.size());
	}
}
