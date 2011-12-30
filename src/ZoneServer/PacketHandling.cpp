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
*                  Zone Server Packet Manipulation          	    *
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
#include "PlayerModules.hpp"

/* TODO
int packet_msgsend(const unsigned char* buf, int len, struct block_list* bl, enum talkarea type)
{
	
	int i;
	
  switch(type) {

 COMMONTALK_AREA:
	// TODO

  }

  return 0;
}*/

/*! 
 *  \brief     Message Format
 *  \details   Format the given message to send it 
 *  to the client.
 *  \author    Fimbulwinter Development Team
 *  \author    Castor
 *  \date      29/12/11
 *
 **/
static bool packet_msgformat(struct ZoneSessionData* zd, char** _name, char** _mes, int type)
{
	char *text, *name, *mes, *_namelen, *_meslen;
	unsigned int packetlen, textlen, namelen, meslen;
	
    *_name = NULL; *_namelen = 0; *_mes = NULL; *_meslen = 0;

	packetlen = RFIFOW(zd->cl,2);
	if( packetlen > RFIFOREST(zd->cl) )
	{	
		ShowWarning("packethandle_msgparse: Incorrect packet length ( '%d' should be '%d' ) received from '%s'\n", packetlen, RFIFOREST(zd->cl), zd->status.name);
		return false;
	}
	
	if( packetlen < 4 + 1 )
	{	
		ShowWarning("packethandle_msgparse: Malformed packet received from '%s' ", zd->status.name );
		return false;
	}

	text = (char*)RFIFOP(zd->cl,4);
	textlen = packetlen - 4;

	if(type){
	
		name = text;
	    namelen = strnlen(zd->status.name , NAME_LENGTH-1);

	    if( strncmp(name, zd->status.name , namelen) || name[namelen] != ' ' || name[namelen+1] != ':' || name[namelen+2] != ' ' ) 
	    {
		  ShowWarning("packethandle_msgparse: Client Desync bug received from '%s'\n", zd->status.name );
		  return false;
	    }

	    mes = name + namelen + 3;
	    meslen = textlen - namelen - 3;
	
}else{ 
		
	  if( textlen < NAME_LENGTH + 1 )
	  {
	    ShowWarning("packethandle_msgparse: Incorrect packet length ( '%d' should be '%d' ) received from '%s'\n", textlen , NAME_LENGTH+1 , zd->status.name);
	    return false;
	  }

	  name = text;
	  namelen = strnlen(name, NAME_LENGTH-1);

	  if( name[namelen] != '\0' )
	  {	
		ShowWarning("packethandle_msgparse: The Character '%s' sent a untermined name ( without '\0' ) \n", zd->status.name );
		return false;
	  }

		mes = name + NAME_LENGTH;
		meslen = textlen - NAME_LENGTH; 
	
}

	if( meslen != strnlen(mes, meslen)+1 )
	{	
		ShowWarning("packethandle_msgparse: Incorrect packet length ( '%d' should be '%d' ) received from '%s'\n", meslen , strnlen(mes, meslen)+1 , zd->status.name );
		return false;		
	}
	
	if( mes[meslen-1] != '\0' )
	{	
        ShowWarning("packethandle_msgparse: The Character '%s' sent a untermined message ( without '\0' ) \n", zd->status.name );
		return false;		
	}
	

	*_name = name;
	*_mes = mes;
	return true;
}

/*! 
 *  \brief     Chat Packets Manipulation
 *  \details   General player talk messages.
 *  \author    Fimbulwinter Development Team
 *  \author    Castor
 *  \date      29/12/11
 *
 **/
void packet_chatpackets(tcp_connection::pointer cl, struct ZoneSessionData* zd, enum TypeChat tc)
{
	const char* text = (char*)RFIFOP(cl,4);
	int textlen = RFIFOW(cl,2) - 4;

	char *name, *message;
	int namelen, messagelen;

	if( !packet_msgformat(zd, &name, &message, 0) )
	  return;

	switch(tc)
	{
		case NORMAL_CHAT:
			WFIFOPACKET(cl, packet, ZC_NOTIFY_CHAT);
			packet->PacketLength = ( sizeof(struct PACKET_ZC_NOTIFY_CHAT) + textlen );
			packet->header = HEADER_ZC_NOTIFY_CHAT;
			packet->GID = zd->bl.id;
			strncpy((char*)WFIFOP(cl,8), text, textlen);
			//TODO: packethandle_msgsend();
			
			memcpy(WFIFOP(cl,0), RFIFOP(cl,0), RFIFOW(cl,2));
			WFIFOPACKET(cl, packet_s , ZC_NOTIFY_PLAYERCHAT);
			packet_s->header = HEADER_ZC_NOTIFY_PLAYERCHAT;
			cl->skip(sizeof(PACKET_ZC_NOTIFY_CHAT));
			

  		/*case PARTY_CHAT:
			// Todo Party Chat

		case GUILD_CHAT:
			// Todo Guild Chat

		case WHISPER_CHAT:
			// Todo Whisper Chat*/
     }

	return;
}