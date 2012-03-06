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

#pragma once

#include  "../Common/show_message.hpp"
#include  "../Common/packets.hpp"


#include <iostream>
#include <cstdlib>

#include "ZoneServer.hpp"

/*! \brief     Talk Area
 *  \author    Fimbulwinter Development Team
 *  \date      31/12/11
 **/
typedef enum talkarea {

	COMMONTALK_AREA = 0,
	
}TalkArea;

/*! \brief     TypeChat
 *  \author    Fimbulwinter Development Team
 *  \date      31/12/11
 **/
typedef enum typechat {

	NORMAL_CHAT = 0,
	PARTY_CHAT,
	GUILD_CHAT,
	WHISPER_CHAT,

}TypeChat;

void packet_ticksend(tcp_connection::pointer cl, ZoneSessionData *sd);
void packet_loadendack(tcp_connection::pointer cl, ZoneSessionData *sd);
void packet_lesseffect(tcp_connection::pointer cl, ZoneSessionData *sd);
void packet_wanttoconnect(tcp_connection::pointer cl, ZoneSessionData *sd);
void packet_chatpackets(tcp_connection::pointer cl, struct ZoneSessionData* zd, enum typechat tc);
