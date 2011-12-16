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
* Function:	Connect to Auth-Server								*                                                     
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

		// TODO: Send auth packets

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
	ZoneSessionData *csd;

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
		
		default:
			ShowWarning("Unknown packet 0x%x sent from AuthServer, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

