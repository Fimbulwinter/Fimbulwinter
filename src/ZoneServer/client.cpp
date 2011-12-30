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
#include "BlockManager.hpp"
#include "PacketHandling.hpp"

static inline void WBUFPOS(unsigned char * p, unsigned short pos, short x, short y, unsigned char dir)
{
	p += pos;
	p[0] = (unsigned char)(x>>2);
	p[1] = (unsigned char)((x<<6) | ((y>>4)&0x3f));
	p[2] = (unsigned char)((y<<4) | (dir&0xf));
}

// client-side: x0+=sx0*0.0625-0.5 and y0+=sy0*0.0625-0.5
static inline void WBUFPOS2(unsigned char * p, unsigned short pos, short x0, short y0, short x1, short y1, unsigned char sx0, unsigned char sy0)
{
	p += pos;
	p[0] = (unsigned char)(x0>>2);
	p[1] = (unsigned char)((x0<<6) | ((y0>>4)&0x3f));
	p[2] = (unsigned char)((y0<<4) | ((x1>>6)&0x0f));
	p[3] = (unsigned char)((x1<<2) | ((y1>>8)&0x03));
	p[4] = (unsigned char)y1;
	p[5] = (unsigned char)((sx0<<4) | (sy0&0x0f));
}

static inline void WFIFOPOS(tcp_connection::pointer cl, unsigned short pos, short x, short y, unsigned char dir)
{
	WBUFPOS(WFIFOP(cl,pos), 0, x, y, dir);
}

inline void WFIFOPOS2(tcp_connection::pointer cl, unsigned short pos, short x0, short y0, short x1, short y1, unsigned char sx0, unsigned char sy0)
{
	WBUFPOS2(WFIFOP(cl,pos), 0, x0, y0, x1, y1, sx0, sy0);
}

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
		{
			BlockManager::remove_block(&sd->bl);
		
			delete sd;
		}

		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

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
				else if(sd && sd->bl.prev == NULL && client_packets[cmd]->callback != packet_loadendack)
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
	int cmd, account_id, char_id, login_id1, sex;
	unsigned int client_tick;

	cmd = RFIFOW(cl,0);
	account_id  = RFIFOL(cl, client_packets[cmd]->pos[0]);
	char_id     = RFIFOL(cl, client_packets[cmd]->pos[1]);
	login_id1   = RFIFOL(cl, client_packets[cmd]->pos[2]);
	client_tick = RFIFOL(cl, client_packets[cmd]->pos[3]);
	sex         = RFIFOB(cl, client_packets[cmd]->pos[4]);

	struct BlockList *bl = BlockManager::get_block(account_id);

	if (bl && bl->type != BL_PC)
	{
		ShowError("packet_wanttoconnect: a non-player object already has id %d, please increase the starting account number.\n", account_id);
		
		WFIFOHEAD(cl, 3);
		WFIFOW(cl, 0) = 0x6a;
		WFIFOB(cl, 2) = 3; // Rejected by server
		cl->send_buffer(3);
		cl->set_eof();
		
		return;
	}

	if (bl || auth_nodes.count(account_id))
	{
		auth_fail(cl, 8);
		return;
	}

	sd = new ZoneSessionData();
	sd->cl = cl;
	cl->set_data((char*)sd);

	PC::set_new_pc(sd, account_id, char_id, login_id1, client_tick, sex, cl);

#if PACKETVER < 20070521
	WFIFOHEAD(cl,4);
	WFIFOL(cl,0) = sd->bl.id;
	cl->send_buffer(4);
#else
	WFIFOHEAD(cl, 6);
	WFIFOW(cl,0) = HEADER_ZC_AID;
	WFIFOL(cl,2) = sd->bl.id;
	cl->send_buffer(6);
#endif

	inter_confirm_auth(sd);
}

void ZoneServer::auth_fail(tcp_connection::pointer cl, int err)
{
	WFIFOHEAD(cl, 3);
	WFIFOW(cl,0) = 0x81;
	WFIFOB(cl,2) = err;
	cl->send_buffer(3);
	cl->set_eof();
}


void ZoneServer::auth_ok( ZoneSessionData * sd )
{
	WFIFOHEAD(sd->cl, 11);
	WFIFOW(sd->cl, 0) = HEADER_ZC_ACCEPT_ENTER;
	WFIFOL(sd->cl, 2) = (unsigned int)time(NULL);
	WFIFOPOS(sd->cl, 6, sd->bl.x, sd->bl.y, 3/*sd->ud.dir*/); // TODO: UnitData Direction
	WFIFOB(sd->cl, 9) = 5;
	WFIFOB(sd->cl,10) = 5;
	sd->cl->send_buffer(11);
}

void ZoneServer::packet_lesseffect(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	int isLess = RFIFOL(cl, client_packets[RFIFOW(cl, 0)]->pos[0]);

	sd->state.lesseffect = (isLess != 0);
}

void ZoneServer::packet_loadendack(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	if(sd->bl.prev != NULL)
		return;

	if (!sd->state.active)
	{
		sd->state.connect_new = 0;

		return;
	}

	// TODO: Update Look, Items, Cart, Guild, Party and Guild
	addblock(&sd->bl);
	clif_spawn(&sd->bl);

	// TODO: Send Map Properties
	// TODO: Info about nearby objects
}

void ZoneServer::packet_ticksend(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	sd->client_tick = RFIFOL(cl, client_packets[RFIFOW(cl, 0)]->pos[0]);

	WFIFOHEAD(cl, 6);
	WFIFOW(cl,0) = 0x7f;
	WFIFOL(cl,2) = (unsigned int)time(NULL);
	cl->send_buffer(6);
}

void ZoneServer::clif_spawn( struct BlockList* bl )
{
	// TODO: this
}

void ZoneServer::init_packets() 
{
	memset(client_packets, 0, sizeof(client_packets));

#if CLIENTVER >= 5
	addpacket(0x007d, 2, &ZoneServer::packet_loadendack, 0);
	addpacket(0x014d, 2, NULL, 0);
	addpacket(0x014f, 6, NULL, 2);
#endif

#if CLIENTVER >= 13
	addpacket(0x021d, 6, &ZoneServer::packet_lesseffect, 2);
#endif

#if CLIENTVER >= 26
	addpacket(0x0436, 19, &ZoneServer::packet_wanttoconnect, 2, 6, 10, 14, 18);
	addpacket(0x0360, 6, &ZoneServer::packet_ticksend, 2);
	addpacket(0x0368, 6, NULL, 2);
	//addpacket(0x035f, 6, &ZoneServer::packet_walktoxy, 2);
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

	if (client_packets[id])
		delete client_packets[id];
	client_packets[id] = pd;
}
