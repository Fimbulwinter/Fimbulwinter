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
#include "BlockManager.hpp"
#include "PacketHandling.hpp"

/*! 
 *  \brief     Zone Server Player Informations
 *  \details   All the zone server informations 
 *  regarding the character can be found here 
 *  \author    Fimbulwinter Development Team
 *  \author    Castor
 *  \date      29/12/11
 *
 **/

struct zonedata {
	
	tcp_connection::pointer cl;

	struct {
         int login_id1; 
		 int login_id2;
		 char name[NAME_LENGTH];
    }playerinfo;

	struct {
		 char message[MESSAGE_SIZE];
    }chatinfo;

	struct {
		struct BlockList bl;
	}mapinfo;

};