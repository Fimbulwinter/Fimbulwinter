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
*                        Client infos Modules              	        *
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
#include <stdarg.h>
#include <stdint.h>

PacketData *ZoneServer::client_packets[MAX_PACKET_DB];

/*==============================================================*
* Function:	Parse from Client									*                                                     
* Author: GreenBox												*
* Date: 08/05/11												*
* Description: Parse informations from client		            *
**==============================================================*/
int ZoneServer::parse_from_client(tcp_connection::pointer cl)
{
	ZoneSessionData *sd = ((ZoneSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		if (sd)
			delete sd;

		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		//#define FIFOSD_CHECK(rest) { if(RFIFOREST(cl) < rest) return 0; if (csd==NULL || !csd->auth) { cl->skip(rest); return 0; } }

		if (client_packets[cmd])
		{
			unsigned short packet_size;

			if (client_packets[cmd]->len > -1)
			{
				packet_size = client_packets[cmd]->len;
			}
			else if (client_packets[cmd]->len == -1)
			{
				if (RFIFOREST(cl) < 4)
					return 0;

				packet_size = RFIFOW(cl, 2);
			}

			if (RFIFOREST(cl) < packet_size)
				return 0;

			if (client_packets[cmd]->callback)
			{
				if (!sd && client_packets[cmd]->callback != &ZoneServer::packet_wanttoconnect)
					;
				else if (sd && cl->flags.eof)
					;
				else
					client_packets[cmd]->callback(cl, sd);
			}

			cl->skip(client_packets[cmd]->len == -1 ? RFIFOW(cl, 2) : client_packets[cmd]->len);
		}
		else
		{
			ShowWarning("Unknown packet 0x%04x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

void ZoneServer::packet_wanttoconnect(tcp_connection::pointer cl, ZoneSessionData *sd)
{


}

void ZoneServer::init_packets() 
{
	memset(client_packets, 0, sizeof(client_packets));

#if CLIENTVER == 26
	addpacket(0x0436, 19, &ZoneServer::packet_wanttoconnect, 2, 6, 10, 14, 18);
#endif
}

void ZoneServer::client_add_packet(unsigned short id, short size, PacketCallback func, ...)
{
	if (id > MAX_PACKET_DB)
	{
		ShowWarning("(ZoneServer::add_packet) invalid packet id %x.\n", id);
		return;
	}

	PacketData *pd = new PacketData();

	va_list va;
	va_start(va, func);

	pd->len = size;
	pd->callback = func;

	for (int i = 0; i < MAX_PACKET_POS; i++)
	{
		pd->pos[i] = va_arg(va, unsigned short);

		if (pd->pos[i] == 0xFFFF)
			break;
	}

	va_end(va);

	client_packets[id] = pd;
}
