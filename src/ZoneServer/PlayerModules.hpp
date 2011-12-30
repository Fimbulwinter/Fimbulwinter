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
*                    Player Manipulation Modules          	        *
* ==================================================================*/

#pragma once

#include <ragnarok.hpp>
#include <packets.hpp>
#include <timers.hpp>
#include <iostream>
#include "BlockManager.hpp"
#include "PacketHandling.hpp"
#include "ZoneServer.hpp"

typedef enum ClearType
{
	CLR_OUTSIGHT = 0,
	CLR_DEAD,
	CLR_RESPAWN,
	CLR_TELEPORT,
} ClearType;

/*! 
 *  \brief     Zone Server Player Informations
 *  \details   All the zone server informations 
 *  regarding the character can be found here 
 *  \author    Fimbulwinter Development Team
 *  \author    Castor
 *  \date      29/12/11
 *
 **/
struct ZoneSessionData
{
	int login_id1;
	int login_id2;
	int gmlevel;
	unsigned int client_tick;
	time_t canlog_tick;

	unsigned short mapindex;
	struct BlockList bl;
	CharData status;
	Registry save_reg;

	struct
	{
		unsigned int lesseffect : 1;
		unsigned int active : 1;
		unsigned int connect_new : 1;
	} state;

	tcp_connection::pointer cl;
};


/*! 
 *  \brief     Player Modules Main Class
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      29/12/11
 *
 **/
class PC
{
public:

	static void set_new_pc(ZoneSessionData *sd, int account_id, int char_id, int login_id1, unsigned int client_tick, int sex, tcp_connection::pointer cl)
	{
		if (!sd)
			return;

		sd->bl.id        = account_id;
		sd->status.account_id   = account_id;
		sd->status.char_id      = char_id;
		sd->status.sex   = sex;
		sd->login_id1    = login_id1;
		sd->login_id2    = 0;
		sd->client_tick  = client_tick;
		sd->bl.type      = BL_PC;
		sd->canlog_tick  = time(NULL);
		sd->state.active = 0;
	}

	static bool auth_ok(ZoneSessionData * sd, unsigned int login_id2, time_t expiration_time, int gmlevel, struct CharData * status);
	static void auth_fail(ZoneSessionData * sd);
	static int set_pos( ZoneSessionData * sd, unsigned short m, short x, short y, ClearType clrtype );
	static void reg_received( struct ZoneSessionData * sd );
};
