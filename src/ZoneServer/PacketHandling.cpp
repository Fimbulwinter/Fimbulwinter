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

/*! 
 *  \brief     Initialize Packets
 * 
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      01/01/12
 *
 **/
void ZoneServer::init_packets() 
{
	memset(client_packets, 0, sizeof(client_packets));

	// mass change regex: 
	// find: NULL,(.*)\); \/\/ loadendack
	// rep: name,\1); // loadendack

#if CLIENTVER >= 5
	addpacket(0x0064, 55, NULL);
	addpacket(0x0065, 17, NULL);
	addpacket(0x0066, 6, NULL);
	addpacket(0x0067, 37, NULL);
	addpacket(0x0068, 46, NULL);
	addpacket(0x0069, -1, NULL);
	addpacket(0x006a, 23, NULL);
	addpacket(0x006b, -1, NULL);
	addpacket(0x006c, 3, NULL);
	addpacket(0x006d, 108, NULL);
	addpacket(0x006e, 3, NULL);
	addpacket(0x006f, 2, NULL);
	addpacket(0x0070, 6, NULL);
	addpacket(0x0071, 28, NULL);
	addpacket(0x0072, 19, packet_wanttoconnect,2,6,10,14,18); // loadendackion
	addpacket(0x0073, 11, NULL);
	addpacket(0x0074, 3, NULL);
	addpacket(0x0075, -1, NULL);
	addpacket(0x0076, 9, NULL);
	addpacket(0x0077, 5, NULL);
	addpacket(0x0078, 54, NULL);
	addpacket(0x0079, 53, NULL);
	addpacket(0x007a, 58, NULL);
	addpacket(0x007b, 60, NULL);
	addpacket(0x007c, 41, NULL);
	addpacket(0x007d, 2, packet_loadendack,0); // loadendack
	addpacket(0x007e, 6, packet_ticksend,2); // loadendack
	addpacket(0x007f, 6, NULL);
	addpacket(0x0080, 7, NULL);
	addpacket(0x0081, 3, NULL);
	addpacket(0x0082, 2, NULL);
	addpacket(0x0083, 2, NULL);
	addpacket(0x0084, 2, NULL);
	addpacket(0x0085, 5, NULL,2); // walktoxy
	addpacket(0x0086, 16, NULL);
	addpacket(0x0087, 12, NULL);
	addpacket(0x0088, 10, NULL);
	addpacket(0x0089, 7, NULL,2,6); // actionrequest
	addpacket(0x008a, 29, NULL);
	addpacket(0x008b, 2, NULL);
	addpacket(0x008c, -1, boost::bind(&packet_chatpackets, _1, _2, NORMAL_CHAT) ,2,4); // globalmessage
	addpacket(0x008d, -1, NULL);
	addpacket(0x008e, -1, NULL);
	//addpacket(0x008f, -1, NULL);
	addpacket(0x0090, 7, NULL,2); // npcclicked
	addpacket(0x0091, 22, NULL);
	addpacket(0x0092, 28, NULL);
	addpacket(0x0093, 2, NULL);
	addpacket(0x0094, 6, NULL,2); // getcharnamerequest
	addpacket(0x0095, 30, NULL);
	addpacket(0x0096, -1, NULL,2,4,28); // wis
	addpacket(0x0097, -1, NULL);
	addpacket(0x0098, 3, NULL);
	addpacket(0x0099, -1, NULL,2,4); // broadcast
	addpacket(0x009a, -1, NULL);
	addpacket(0x009b, 5, NULL,2,4); // changedir
	addpacket(0x009c, 9, NULL);
	addpacket(0x009d, 17, NULL);
	addpacket(0x009e, 17, NULL);
	addpacket(0x009f, 6, NULL,2); // takeitem
	addpacket(0x00a0, 23, NULL);
	addpacket(0x00a1, 6, NULL);
	addpacket(0x00a2, 6, NULL,2,4); // dropitem
	addpacket(0x00a3, -1, NULL);
	addpacket(0x00a4, -1, NULL);
	addpacket(0x00a5, -1, NULL);
	addpacket(0x00a6, -1, NULL);
	addpacket(0x00a7, 8, NULL,2,4); // useitem
	addpacket(0x00a8, 7, NULL);
	addpacket(0x00a9, 6, NULL,2,4); // equipitem
	addpacket(0x00aa, 7, NULL);
	addpacket(0x00ab, 4, NULL,2); // unequipitem
	addpacket(0x00ac, 7, NULL);
	//addpacket(0x00ad, -1, NULL);
	addpacket(0x00ae, -1, NULL);
	addpacket(0x00af, 6, NULL);
	addpacket(0x00b0, 8, NULL);
	addpacket(0x00b1, 8, NULL);
	addpacket(0x00b2, 3, NULL,2); // restart
	addpacket(0x00b3, 3, NULL);
	addpacket(0x00b4, -1, NULL);
	addpacket(0x00b5, 6, NULL);
	addpacket(0x00b6, 6, NULL);
	addpacket(0x00b7, -1, NULL);
	addpacket(0x00b8, 7, NULL,2,6); // npcselectmenu
	addpacket(0x00b9, 6, NULL,2); // npcnextclicked
	addpacket(0x00ba, 2, NULL);
	addpacket(0x00bb, 5, NULL,2,4); // statusup
	addpacket(0x00bc, 6, NULL);
	addpacket(0x00bd, 44, NULL);
	addpacket(0x00be, 5, NULL);
	addpacket(0x00bf, 3, NULL,2); // emotion
	addpacket(0x00c0, 7, NULL);
	addpacket(0x00c1, 2, NULL,0); // howmanyconnections
	addpacket(0x00c2, 6, NULL);
	addpacket(0x00c3, 8, NULL);
	addpacket(0x00c4, 6, NULL);
	addpacket(0x00c5, 7, NULL,2,6); // npcbuysellselected
	addpacket(0x00c6, -1, NULL);
	addpacket(0x00c7, -1, NULL);
	addpacket(0x00c8, -1, NULL,2,4); // npcbuylistsend
	addpacket(0x00c9, -1, NULL,2,4); // npcselllistsend
	addpacket(0x00ca, 3, NULL);
	addpacket(0x00cb, 3, NULL);
	addpacket(0x00cc, 6, NULL,2); // gmkick
	addpacket(0x00cd, 3, NULL);
	addpacket(0x00ce, 2, NULL,0); // killall
	addpacket(0x00cf, 27, NULL,2,26); // wisexin
	addpacket(0x00d0, 3, NULL,2); // wisall
	addpacket(0x00d1, 4, NULL);
	addpacket(0x00d2, 4, NULL);
	addpacket(0x00d3, 2, NULL,0); // wisexlist
	addpacket(0x00d4, -1, NULL);
	addpacket(0x00d5, -1, NULL,2,4,6,7,15); // createchatroom
	addpacket(0x00d6, 3, NULL);
	addpacket(0x00d7, -1, NULL);
	addpacket(0x00d8, 6, NULL);
	addpacket(0x00d9, 14, NULL,2,6); // chataddmember
	addpacket(0x00da, 3, NULL);
	addpacket(0x00db, -1, NULL);
	addpacket(0x00dc, 28, NULL);
	addpacket(0x00dd, 29, NULL);
	addpacket(0x00de, -1, NULL,2,4,6,7,15); // chatroomstatuschange
	addpacket(0x00df, -1, NULL);
	addpacket(0x00e0, 30, NULL,2,6); // changechatowner
	addpacket(0x00e1, 30, NULL);
	addpacket(0x00e2, 26, NULL,2); // kickfromchat
	addpacket(0x00e3, 2, NULL,0); // chatleave
	addpacket(0x00e4, 6, NULL,2); // traderequest
	addpacket(0x00e5, 26, NULL);
	addpacket(0x00e6, 3, NULL,2); // tradeack
	addpacket(0x00e7, 3, NULL);
	addpacket(0x00e8, 8, NULL,2,4); // tradeadditem
	addpacket(0x00e9, 19, NULL);
	addpacket(0x00ea, 5, NULL);
	addpacket(0x00eb, 2, NULL,0); // tradeok
	addpacket(0x00ec, 3, NULL);
	addpacket(0x00ed, 2, NULL,0); // tradecancel
	addpacket(0x00ee, 2, NULL);
	addpacket(0x00ef, 2, NULL,0); // tradecommit
	addpacket(0x00f0, 3, NULL);
	addpacket(0x00f1, 2, NULL);
	addpacket(0x00f2, 6, NULL);
	addpacket(0x00f3, 8, NULL,2,4); // movetokafra
	addpacket(0x00f4, 21, NULL);
	addpacket(0x00f5, 8, NULL,2,4); // movefromkafra
	addpacket(0x00f6, 8, NULL);
	addpacket(0x00f7, 2, NULL,0); // closekafra
	addpacket(0x00f8, 2, NULL);
	addpacket(0x00f9, 26, NULL,2); // createparty
	addpacket(0x00fa, 3, NULL);
	addpacket(0x00fb, -1, NULL);
	addpacket(0x00fc, 6, NULL,2); // partyinvite
	addpacket(0x00fd, 27, NULL);
	addpacket(0x00fe, 30, NULL);
	addpacket(0x00ff, 10, NULL,2,6); // replypartyinvite
	addpacket(0x0100, 2, NULL,0); // leaveparty
	addpacket(0x0101, 6, NULL);
	addpacket(0x0102, 6, NULL,2,4); // partychangeoption
	addpacket(0x0103, 30, NULL,2,6); // removepartymember
	addpacket(0x0104, 79, NULL);
	addpacket(0x0105, 31, NULL);
	addpacket(0x0106, 10, NULL);
	addpacket(0x0107, 10, NULL);
	addpacket(0x0108, -1, NULL,2,4); // partymessage
	addpacket(0x0109, -1, NULL);
	addpacket(0x010a, 4, NULL);
	addpacket(0x010b, 6, NULL);
	addpacket(0x010c, 6, NULL);
	addpacket(0x010d, 2, NULL);
	addpacket(0x010e, 11, NULL);
	addpacket(0x010f, -1, NULL);
	addpacket(0x0110, 10, NULL);
	addpacket(0x0111, 39, NULL);
	addpacket(0x0112, 4, NULL,2); // skillup
	addpacket(0x0113, 10, NULL,2,4,6); // useskilltoid
	addpacket(0x0114, 31, NULL);
	addpacket(0x0115, 35, NULL);
	addpacket(0x0116, 10, NULL,2,4,6,8); // useskilltopos
	addpacket(0x0117, 18, NULL);
	addpacket(0x0118, 2, NULL,0); // stopattack
	addpacket(0x0119, 13, NULL);
	addpacket(0x011a, 15, NULL);
	addpacket(0x011b, 20, NULL,2,4); // useskillmap
	addpacket(0x011c, 68, NULL);
	addpacket(0x011d, 2, NULL,0); // requestmemo
	addpacket(0x011e, 3, NULL);
	addpacket(0x011f, 16, NULL);
	addpacket(0x0120, 6, NULL);
	addpacket(0x0121, 14, NULL);
	addpacket(0x0122, -1, NULL);
	addpacket(0x0123, -1, NULL);
	addpacket(0x0124, 21, NULL);
	addpacket(0x0125, 8, NULL);
	addpacket(0x0126, 8, NULL,2,4); // putitemtocart
	addpacket(0x0127, 8, NULL,2,4); // getitemfromcart
	addpacket(0x0128, 8, NULL,2,4); // movefromkafratocart
	addpacket(0x0129, 8, NULL,2,4); // movetokafrafromcart
	addpacket(0x012a, 2, NULL,0); // removeoption
	addpacket(0x012b, 2, NULL);
	addpacket(0x012c, 3, NULL);
	addpacket(0x012d, 4, NULL);
	addpacket(0x012e, 2, NULL,0); // closevending
	addpacket(0x012f, -1, NULL);
	addpacket(0x0130, 6, NULL,2); // vendinglistreq
	addpacket(0x0131, 86, NULL);
	addpacket(0x0132, 6, NULL);
	addpacket(0x0133, -1, NULL);
	addpacket(0x0134, -1, NULL,2,4,8); // purchasereq
	addpacket(0x0135, 7, NULL);
	addpacket(0x0136, -1, NULL);
	addpacket(0x0137, 6, NULL);
	addpacket(0x0138, 3, NULL);
	addpacket(0x0139, 16, NULL);
	addpacket(0x013a, 4, NULL);
	addpacket(0x013b, 4, NULL);
	addpacket(0x013c, 4, NULL);
	addpacket(0x013d, 6, NULL);
	addpacket(0x013e, 24, NULL);
	addpacket(0x013f, 26, NULL,2); // itemmonster
	addpacket(0x0140, 22, NULL,2,18,20); // mapmove
	addpacket(0x0141, 14, NULL);
	addpacket(0x0142, 6, NULL);
	addpacket(0x0143, 10, NULL,2,6); // npcamountinput
	addpacket(0x0144, 23, NULL);
	addpacket(0x0145, 19, NULL);
	addpacket(0x0146, 6, NULL,2); // npccloseclicked
	addpacket(0x0147, 39, NULL);
	addpacket(0x0148, 8, NULL);
	addpacket(0x0149, 9, NULL,2,6,7); // gmreqnochat
	addpacket(0x014a, 6, NULL);
	addpacket(0x014b, 27, NULL);
	addpacket(0x014c, -1, NULL);
	addpacket(0x014d, 2, NULL,0); // guildcheckmaster
	addpacket(0x014e, 6, NULL);
	addpacket(0x014f, 6, NULL,2); // guildrequestinfo
	addpacket(0x0150, 110, NULL);
	addpacket(0x0151, 6, NULL,2); // guildrequestemblem
	addpacket(0x0152, -1, NULL);
	addpacket(0x0153, -1, NULL,2,4); // guildchangeemblem
	addpacket(0x0154, -1, NULL);
	addpacket(0x0155, -1, NULL,2); // guildchangememberposition
	addpacket(0x0156, -1, NULL);
	addpacket(0x0157, 6, NULL);
	addpacket(0x0158, -1, NULL);
	addpacket(0x0159, 54, NULL,2,6,10,14); // guildleave
	addpacket(0x015a, 66, NULL);
	addpacket(0x015b, 54, NULL,2,6,10,14); // guildexpulsion
	addpacket(0x015c, 90, NULL);
	addpacket(0x015d, 42, NULL,2); // guildbreak
	addpacket(0x015e, 6, NULL);
	addpacket(0x015f, 42, NULL);
	addpacket(0x0160, -1, NULL);
	addpacket(0x0161, -1, NULL,2); // guildchangepositioninfo
	addpacket(0x0162, -1, NULL);
	addpacket(0x0163, -1, NULL);
	addpacket(0x0164, -1, NULL);
	addpacket(0x0165, 30, NULL,6); // createguild
	addpacket(0x0166, -1, NULL);
	addpacket(0x0167, 3, NULL);
	addpacket(0x0168, 14, NULL,2); // guildinvite
	addpacket(0x0169, 3, NULL);
	addpacket(0x016a, 30, NULL);
	addpacket(0x016b, 10, NULL,2,6); // guildreplyinvite
	addpacket(0x016c, 43, NULL);
	addpacket(0x016d, 14, NULL);
	addpacket(0x016e, 186, NULL,2,6,66); // guildchangenotice
	addpacket(0x016f, 182, NULL);
	addpacket(0x0170, 14, NULL,2); // guildrequestalliance
	addpacket(0x0171, 30, NULL);
	addpacket(0x0172, 10, NULL,2,6); // guildreplyalliance
	addpacket(0x0173, 3, NULL);
	addpacket(0x0174, -1, NULL);
	addpacket(0x0175, 6, NULL);
	addpacket(0x0176, 106, NULL);
	addpacket(0x0177, -1, NULL);
	addpacket(0x0178, 4, NULL,2); // itemidentify
	addpacket(0x0179, 5, NULL);
	addpacket(0x017a, 4, NULL,2); // usecard
	addpacket(0x017b, -1, NULL);
	addpacket(0x017c, 6, NULL,2,4); // insertcard
	addpacket(0x017d, 7, NULL);
	addpacket(0x017e, -1, NULL,2,4); // guildmessage
	addpacket(0x017f, -1, NULL);
	addpacket(0x0180, 6, NULL,2); // guildopposition
	addpacket(0x0181, 3, NULL);
	addpacket(0x0182, 106, NULL);
	addpacket(0x0183, 10, NULL,2,6); // guilddelalliance
	addpacket(0x0184, 10, NULL);
	addpacket(0x0185, 34, NULL);
	//addpacket(0x0186, -1, NULL);
	addpacket(0x0187, 6, NULL);
	addpacket(0x0188, 8, NULL);
	addpacket(0x0189, 4, NULL);
	addpacket(0x018a, 4, NULL,0); // quitgame
	addpacket(0x018b, 4, NULL);
	addpacket(0x018c, 29, NULL);
	addpacket(0x018d, -1, NULL);
	addpacket(0x018e, 10, NULL,2,4,6,8); // producemix
	addpacket(0x018f, 6, NULL);
	addpacket(0x0190, 90, NULL,2,4,6,8,10); // useskilltoposinfo
	addpacket(0x0191, 86, NULL);
	addpacket(0x0192, 24, NULL);
	addpacket(0x0193, 6, NULL,2); // solvecharname
	addpacket(0x0194, 30, NULL);
	addpacket(0x0195, 102, NULL);
	addpacket(0x0196, 9, NULL);
	addpacket(0x0197, 4, NULL,2); // resetchar
	addpacket(0x0198, 8, NULL,2,4,6); // changemaptype
	addpacket(0x0199, 4, NULL);
	addpacket(0x019a, 14, NULL);
	addpacket(0x019b, 10, NULL);
	addpacket(0x019c, -1, NULL,2,4); // localbroadcast
	addpacket(0x019d, 6, NULL,0); // gmhide
	addpacket(0x019e, 2, NULL);
	addpacket(0x019f, 6, NULL,2); // catchpet
	addpacket(0x01a0, 3, NULL);
	addpacket(0x01a1, 3, NULL,2); // petmenu
	addpacket(0x01a2, 35, NULL);
	addpacket(0x01a3, 5, NULL);
	addpacket(0x01a4, 11, NULL);
	addpacket(0x01a5, 26, NULL,2); // changepetname
	addpacket(0x01a6, -1, NULL);
	addpacket(0x01a7, 4, NULL,2); // selectegg
	addpacket(0x01a8, 4, NULL);
	addpacket(0x01a9, 6, NULL,2); // sendemotion
	addpacket(0x01aa, 10, NULL);
	addpacket(0x01ab, 12, NULL);
	addpacket(0x01ac, 6, NULL);
	addpacket(0x01ad, -1, NULL);
	addpacket(0x01ae, 4, NULL,2); // selectarrow
	addpacket(0x01af, 4, NULL,2); // changecart
	addpacket(0x01b0, 11, NULL);
	addpacket(0x01b1, 7, NULL);
	addpacket(0x01b2, -1, NULL,2,4,84,85); // openvending
	addpacket(0x01b3, 67, NULL);
	addpacket(0x01b4, 12, NULL);
	addpacket(0x01b5, 18, NULL);
	addpacket(0x01b6, 114, NULL);
	addpacket(0x01b7, 6, NULL);
	addpacket(0x01b8, 3, NULL);
	addpacket(0x01b9, 6, NULL);
	addpacket(0x01ba, 26, NULL,2); // remove
	addpacket(0x01bb, 26, NULL,2); // shift
	addpacket(0x01bc, 26, NULL,2); // recall
	addpacket(0x01bd, 26, NULL,2); // summon
	addpacket(0x01be, 2, NULL);
	addpacket(0x01bf, 3, NULL);
	addpacket(0x01c0, 2, NULL);
	addpacket(0x01c1, 14, NULL);
	addpacket(0x01c2, 10, NULL);
	addpacket(0x01c3, -1, NULL);
	addpacket(0x01c4, 22, NULL);
	addpacket(0x01c5, 22, NULL);
	addpacket(0x01c6, 4, NULL);
	addpacket(0x01c7, 2, NULL);
	addpacket(0x01c8, 13, NULL);
	addpacket(0x01c9, 97, NULL);
	//addpacket(0x01ca, -1, NULL);
	addpacket(0x01cb, 9, NULL);
	addpacket(0x01cc, 9, NULL);
	addpacket(0x01cd, 30, NULL);
	addpacket(0x01ce, 6, NULL,2); // autospell
	addpacket(0x01cf, 28, NULL);
	addpacket(0x01d0, 8, NULL);
	addpacket(0x01d1, 14, NULL);
	addpacket(0x01d2, 10, NULL);
	addpacket(0x01d3, 35, NULL);
	addpacket(0x01d4, 6, NULL);
	addpacket(0x01d5, -1, NULL,2,4,8); // npcstringinput
	addpacket(0x01d6, 4, NULL);
	addpacket(0x01d7, 11, NULL);
	addpacket(0x01d8, 54, NULL);
	addpacket(0x01d9, 53, NULL);
	addpacket(0x01da, 60, NULL);
	addpacket(0x01db, 2, NULL);
	addpacket(0x01dc, -1, NULL);
	addpacket(0x01dd, 47, NULL);
	addpacket(0x01de, 33, NULL);
	addpacket(0x01df, 6, NULL,2); // gmreqaccname
	addpacket(0x01e0, 30, NULL);
	addpacket(0x01e1, 8, NULL);
	addpacket(0x01e2, 34, NULL);
	addpacket(0x01e3, 14, NULL);
	addpacket(0x01e4, 2, NULL);
	addpacket(0x01e5, 6, NULL);
	addpacket(0x01e6, 26, NULL);
	addpacket(0x01e7, 2, NULL,0); // sndoridori
	addpacket(0x01e8, 28, NULL,2); // createparty2
	addpacket(0x01e9, 81, NULL);
	addpacket(0x01ea, 6, NULL);
	addpacket(0x01eb, 10, NULL);
	addpacket(0x01ec, 26, NULL);
	addpacket(0x01ed, 2, NULL,0); // snexplosionspirits
	addpacket(0x01ee, -1, NULL);
	addpacket(0x01ef, -1, NULL);
	addpacket(0x01f0, -1, NULL);
	addpacket(0x01f1, -1, NULL);
	addpacket(0x01f2, 20, NULL);
	addpacket(0x01f3, 10, NULL);
	addpacket(0x01f4, 32, NULL);
	addpacket(0x01f5, 9, NULL);
	addpacket(0x01f6, 34, NULL);
	addpacket(0x01f7, 14, NULL,0); // adoptreply
	addpacket(0x01f8, 2, NULL);
	addpacket(0x01f9, 6, NULL,0); // adoptrequest
	addpacket(0x01fa, 48, NULL);
	addpacket(0x01fb, 56, NULL);
	addpacket(0x01fc, -1, NULL);
	addpacket(0x01fd, 4, NULL,2); // repairitem
	addpacket(0x01fe, 5, NULL);
	addpacket(0x01ff, 10, NULL);
	addpacket(0x0200, 26, NULL);
	addpacket(0x0201, -1, NULL);
	addpacket(0x0202, 26, NULL,2); // friendslistadd
	addpacket(0x0203, 10, NULL,2,6); // friendslistremove
	addpacket(0x0204, 18, NULL);
	addpacket(0x0205, 26, NULL);
	addpacket(0x0206, 11, NULL);
	addpacket(0x0207, 34, NULL);
	addpacket(0x0208, 11, NULL,2,6,10); // friendslistreply
	addpacket(0x0209, 36, NULL);
	addpacket(0x020a, 10, NULL);
	//addpacket(0x020b, -1, NULL);
	//addpacket(0x020c, -1, NULL);
	addpacket(0x020d, -1, NULL);
#endif

	//2004-07-05aSakexe
#if CLIENTVER >= 6
	addpacket(0x0072, 22, packet_wanttoconnect,5,9,13,17,21); // loadendackion
	addpacket(0x0085, 8, NULL,5); // walktoxy
	addpacket(0x00a7, 13, NULL,5,9); // useitem
	addpacket(0x0113, 15, NULL,4,9,11); // useskilltoid
	addpacket(0x0116, 15, NULL,4,9,11,13); // useskilltopos
	addpacket(0x0190, 95, NULL,4,9,11,13,15); // useskilltoposinfo
	addpacket(0x0208, 14, NULL,2,6,10); // friendslistreply
	addpacket(0x020e, 24, NULL);
#endif

	//2004-07-13aSakexe
#if CLIENTVER >= 7
	addpacket(0x0072, 39, packet_wanttoconnect,12,22,30,34,38); // loadendackion
	addpacket(0x0085, 9, NULL,6); // walktoxy
	addpacket(0x009b, 13, NULL,5,12); // changedir
	addpacket(0x009f, 10, NULL,6); // takeitem
	addpacket(0x00a7, 17, NULL,6,13); // useitem
	addpacket(0x0113, 19, NULL,7,9,15); // useskilltoid
	addpacket(0x0116, 19, NULL,7,9,15,17); // useskilltopos
	addpacket(0x0190, 99, NULL,7,9,15,17,19); // useskilltoposinfo
#endif

	//2004-07-26aSakexe
#if CLIENTVER >= 8
	addpacket(0x0072, 14, NULL,5,12); // dropitem
	addpacket(0x007e, 33, packet_wanttoconnect,12,18,24,28,32); // loadendackion
	addpacket(0x0085, 20, NULL,7,12,16); // useskilltoid
	addpacket(0x0089, 15, NULL,11); // getcharnamerequest
	addpacket(0x008c, 23, NULL,3,6,17,21); // useskilltopos
	addpacket(0x0094, 10, NULL,6); // takeitem
	addpacket(0x009b, 6, NULL,3); // walktoxy
	addpacket(0x009f, 13, NULL,5,12); // changedir
	addpacket(0x00a2, 103, NULL,3,6,17,21,23); // useskilltoposinfo
	addpacket(0x00a7, 12, NULL,8); // solvecharname
	addpacket(0x00f3, -1, boost::bind(&packet_chatpackets, _1, _2, NORMAL_CHAT)); // globalmessage
	addpacket(0x00f5, 17, NULL,6,12); // useitem
	addpacket(0x00f7, 10, packet_ticksend,6); // loadendack
	addpacket(0x0113, 16, NULL,5,12); // movetokafra
	addpacket(0x0116, 2, NULL,0); // closekafra
	addpacket(0x0190, 26, NULL,10,22); // movefromkafra
	addpacket(0x0193, 9, NULL,3,8); // actionrequest
#endif

	//2004-08-09aSakexe
#if CLIENTVER >= 9
	addpacket(0x0072, 17, NULL,8,15); // dropitem
	addpacket(0x007e, 37, packet_wanttoconnect,9,21,28,32,36); // loadendackion
	addpacket(0x0085, 26, NULL,11,18,22); // useskilltoid
	addpacket(0x0089, 12, NULL,8); // getcharnamerequest
	addpacket(0x008c, 40, NULL,5,15,29,38); // useskilltopos
	addpacket(0x0094, 13, NULL,9); // takeitem
	addpacket(0x009b, 15, NULL,12); // walktoxy
	addpacket(0x009f, 12, NULL,7,11); // changedir
	addpacket(0x00a2, 120, NULL,5,15,29,38,40); // useskilltoposinfo
	addpacket(0x00a7, 11, NULL,7); // solvecharname
	addpacket(0x00f5, 24, NULL,9,20); // useitem
	addpacket(0x00f7, 13, packet_ticksend,9); // loadendack
	addpacket(0x0113, 23, NULL,5,19); // movetokafra
	addpacket(0x0190, 26, NULL,11,22); // movefromkafra
	addpacket(0x0193, 18, NULL,7,17); // actionrequest

	//2004-08-16aSakexe
	addpacket(0x0212, 26, NULL,2); // rc
	addpacket(0x0213, 26, NULL,2); // check
	addpacket(0x0214, 42, NULL);

	//2004-08-17aSakexe
	addpacket(0x020f, 10, NULL,2,6); // pvpinfo
	addpacket(0x0210, 22, NULL);
#endif

	//2004-09-06aSakexe
#if CLIENTVER >= 10
	addpacket(0x0072, 20, NULL,9,20); // useitem
	addpacket(0x007e, 19, NULL,3,15); // movetokafra
	addpacket(0x0085, 23, NULL,9,22); // actionrequest
	addpacket(0x0089, 9, NULL,6); // walktoxy
	addpacket(0x008c, 105, NULL,10,14,18,23,25); // useskilltoposinfo
	addpacket(0x0094, 17, NULL,6,15); // dropitem
	addpacket(0x009b, 14, NULL,10); // getcharnamerequest
	addpacket(0x009f, -1, boost::bind(packet_chatpackets, _1, _2, NORMAL_CHAT)); // globalmessage
	addpacket(0x00a2, 14, NULL,10); // solvecharname
	addpacket(0x00a7, 25, NULL,10,14,18,23); // useskilltopos
	addpacket(0x00f3, 10, NULL,4,9); // changedir
	addpacket(0x00f5, 34, packet_wanttoconnect,7,15,25,29,33); // loadendackion
	addpacket(0x00f7, 2, NULL,0); // closekafra
	addpacket(0x0113, 11, NULL,7); // takeitem
	addpacket(0x0116, 11, packet_ticksend,7); // loadendack
	addpacket(0x0190, 22, NULL,9,15,18); // useskilltoid
	addpacket(0x0193, 17, NULL,3,13); // movefromkafra
#endif

	//2004-09-20aSakexe
#if CLIENTVER >= 11
	addpacket(0x0072, 18, NULL,10,14); // useitem
	addpacket(0x007e, 25, NULL,6,21); // movetokafra
	addpacket(0x0085, 9, NULL,3,8); // actionrequest
	addpacket(0x0089, 14, NULL,11); // walktoxy
	addpacket(0x008c, 109, NULL,16,20,23,27,29); // useskilltoposinfo
	addpacket(0x0094, 19, NULL,12,17); // dropitem
	addpacket(0x009b, 10, NULL,6); // getcharnamerequest
	addpacket(0x00a2, 10, NULL,6); // solvecharname
	addpacket(0x00a7, 29, NULL,6,20,23,27); // useskilltopos
	addpacket(0x00f3, 18, NULL,8,17); // changedir
	addpacket(0x00f5, 32, packet_wanttoconnect,10,17,23,27,31); // loadendackion
	addpacket(0x0113, 14, NULL,10); // takeitem
	addpacket(0x0116, 14, packet_ticksend,10); // loadendack
	addpacket(0x0190, 14, NULL,4,7,10); // useskilltoid
	addpacket(0x0193, 12, NULL,4,8); // movefromkafra
#endif

	//2004-10-05aSakexe
#if CLIENTVER >= 12
	addpacket(0x0072, 17, NULL,6,13); // useitem
	addpacket(0x007e, 16, NULL,5,12); // movetokafra
	addpacket(0x0089, 6, NULL,3); // walktoxy
	addpacket(0x008c, 103, NULL,2,6,17,21,23); // useskilltoposinfo
	addpacket(0x0094, 14, NULL,5,12); // dropitem
	addpacket(0x009b, 15, NULL,11); // getcharnamerequest
	addpacket(0x00a2, 12, NULL,8); // solvecharname
	addpacket(0x00a7, 23, NULL,3,6,17,21); // useskilltopos
	addpacket(0x00f3, 13, NULL,5,12); // changedir
	addpacket(0x00f5, 33, packet_wanttoconnect,12,18,24,28,32); // loadendackion
	addpacket(0x0113, 10, NULL,6); // takeitem
	addpacket(0x0116, 10, packet_ticksend,6); // loadendack
	addpacket(0x0190, 20, NULL,7,12,16); // useskilltoid
	addpacket(0x0193, 26, NULL,10,22); // movefromkafra
#endif

	//2004-10-25aSakexe
#if CLIENTVER >= 13
	addpacket(0x0072, 13, NULL,5,9); // useitem
	addpacket(0x007e, 13, NULL,6,9); // movetokafra
	addpacket(0x0085, 15, NULL,4,14); // actionrequest
	addpacket(0x008c, 108, NULL,6,9,23,26,28); // useskilltoposinfo
	addpacket(0x0094, 12, NULL,6,10); // dropitem
	addpacket(0x009b, 10, NULL,6); // getcharnamerequest
	addpacket(0x00a2, 16, NULL,12); // solvecharname
	addpacket(0x00a7, 28, NULL,6,9,23,26); // useskilltopos
	addpacket(0x00f3, 15, NULL,6,14); // changedir
	addpacket(0x00f5, 29, packet_wanttoconnect,5,14,20,24,28); // loadendackion
	addpacket(0x0113, 9, NULL,5); // takeitem
	addpacket(0x0116, 9, packet_ticksend,5); // loadendack
	addpacket(0x0190, 26, NULL,4,10,22); // useskilltoid
	addpacket(0x0193, 22, NULL,12,18); // movefromkafra

	//2004-11-01aSakexe
	addpacket(0x0084, -1, NULL);
	addpacket(0x0215, 6, NULL);

	//2004-11-08aSakexe
	addpacket(0x0084, 2, NULL);
	addpacket(0x0216, 6, NULL);
	addpacket(0x0217, 2, NULL,0); // blacksmith
	addpacket(0x0218, 2, NULL,0); // alchemist
	addpacket(0x0219, 282, NULL);
	addpacket(0x021a, 282, NULL);
	addpacket(0x021b, 10, NULL);
	addpacket(0x021c, 10, NULL);

	//2004-11-15aSakexe
	addpacket(0x021d, 6, packet_lesseffect,2); // loadendack
#endif

	//2004-11-29aSakexe
#if CLIENTVER >= 14
	addpacket(0x0072, 22, NULL,8,12,18); // useskilltoid
	addpacket(0x007e, 30, NULL,4,9,22,28); // useskilltopos
	addpacket(0x0085, -1, boost::bind(packet_chatpackets, _1, _2, NORMAL_CHAT)); // globalmessage
	addpacket(0x0089, 7, packet_ticksend,3); // loadendack
	addpacket(0x008c, 13, NULL,9); // getcharnamerequest
	addpacket(0x0094, 14, NULL,4,10); // movetokafra
	addpacket(0x009b, 2, NULL,0); // closekafra
	addpacket(0x009f, 18, NULL,6,17); // actionrequest
	addpacket(0x00a2, 7, NULL,3); // takeitem
	addpacket(0x00a7, 7, NULL,4); // walktoxy
	addpacket(0x00f3, 8, NULL,3,7); // changedir
	addpacket(0x00f5, 29, packet_wanttoconnect,3,10,20,24,28); // loadendackion
	addpacket(0x00f7, 14, NULL,10); // solvecharname
	addpacket(0x0113, 110, NULL,4,9,22,28,30); // useskilltoposinfo
	addpacket(0x0116, 12, NULL,4,10); // dropitem
	addpacket(0x0190, 15, NULL,3,11); // useitem
	addpacket(0x0193, 21, NULL,4,17); // movefromkafra
	addpacket(0x0221, -1, NULL);
	addpacket(0x0222, 6, NULL,2); // weaponrefine
	addpacket(0x0223, 8, NULL);

	//2004-12-13aSakexe
	//skipped, many packets being set to -1
	addpacket(0x0066, 3, NULL);
	addpacket(0x0070, 3, NULL);
	addpacket(0x01ca, 3, NULL);
	addpacket(0x021e, 6, NULL);
	addpacket(0x021f, 66, NULL);
	addpacket(0x0220, 10, NULL);
#endif

	//2005-01-10bSakexe
#if CLIENTVER >= 15
	addpacket(0x0072, 26, NULL,8,16,22); // useskilltoid
	addpacket(0x007e, 114, NULL,10,18,22,32,34); // useskilltoposinfo
	addpacket(0x0085, 23, NULL,12,22); // changedir
	addpacket(0x0089, 9, packet_ticksend,5); // loadendack
	addpacket(0x008c, 8, NULL,4); // getcharnamerequest
	addpacket(0x0094, 20, NULL,10,16); // movetokafra
	addpacket(0x009b, 32, packet_wanttoconnect,3,12,23,27,31); // loadendackion
	addpacket(0x009f, 17, NULL,5,13); // useitem
	addpacket(0x00a2, 11, NULL,7); // solvecharname
	addpacket(0x00a7, 13, NULL,10); // walktoxy
	addpacket(0x00f3, -1, boost::bind(packet_chatpackets, _1, _2, NORMAL_CHAT)); // globalmessage
	addpacket(0x00f5, 9, NULL,5); // takeitem
	addpacket(0x00f7, 21, NULL,11,17); // movefromkafra
	addpacket(0x0113, 34, NULL,10,18,22,32); // useskilltopos
	addpacket(0x0116, 20, NULL,15,18); // dropitem
	addpacket(0x0190, 20, NULL,9,19); // actionrequest
	addpacket(0x0193, 2, NULL,0); // closekafra

	//2005-03-28aSakexe
	addpacket(0x0224, 10, NULL);
	addpacket(0x0225, 2, NULL,0); // taekwon
	addpacket(0x0226, 282, NULL);

	//2005-04-04aSakexe
	addpacket(0x0227, 18, NULL);
	addpacket(0x0228, 18, NULL);

	//2005-04-11aSakexe
	addpacket(0x0229, 15, NULL);
	addpacket(0x022a, 58, NULL);
	addpacket(0x022b, 57, NULL);
	addpacket(0x022c, 64, NULL);

	//2005-04-25aSakexe
	addpacket(0x022d, 5, NULL,4); // hommenu
	addpacket(0x0232, 9, NULL,6); // hommoveto
	addpacket(0x0233, 11, NULL,0); // homattack
	addpacket(0x0234, 6, NULL,0); // hommovetomaster
#endif

	//2005-05-09aSakexe
#if CLIENTVER >= 16
	addpacket(0x0072, 25, NULL,6,10,21); // useskilltoid
	addpacket(0x007e, 102, NULL,5,9,12,20,22); // useskilltoposinfo
	addpacket(0x0085, 11, NULL,7,10); // changedir
	addpacket(0x0089, 8, packet_ticksend,4); // loadendack
	addpacket(0x008c, 11, NULL,7); // getcharnamerequest
	addpacket(0x0094, 14, NULL,7,10); // movetokafra
	addpacket(0x009b, 26, packet_wanttoconnect,4,9,17,21,25); // loadendackion
	addpacket(0x009f, 14, NULL,4,10); // useitem
	addpacket(0x00a2, 15, NULL,11); // solvecharname
	addpacket(0x00a7, 8, NULL,5); // walktoxy
	addpacket(0x00f5, 8, NULL,4); // takeitem
	addpacket(0x00f7, 22, NULL,14,18); // movefromkafra
	addpacket(0x0113, 22, NULL,5,9,12,20); // useskilltopos
	addpacket(0x0116, 10, NULL,5,8); // dropitem
	addpacket(0x0190, 19, NULL,5,18); // actionrequest

	//2005-05-23aSakexe
	addpacket(0x022e, 69, NULL);
	addpacket(0x0230, 12, NULL);

	//2005-05-30aSakexe
	addpacket(0x022e, 71, NULL);
	addpacket(0x0235, -1, NULL);
	addpacket(0x0236, 10, NULL);
	addpacket(0x0237, 2, NULL,0); // rankingpk
	addpacket(0x0238, 282, NULL);

	//2005-05-31aSakexe
	addpacket(0x0216, 2, NULL);
	addpacket(0x0239, 11, NULL);

	//2005-06-08aSakexe
	addpacket(0x0216, 6, NULL);
	addpacket(0x0217, 2, NULL,0); // blacksmith
	addpacket(0x022f, 5, NULL);
	addpacket(0x0231, 26, NULL,0); // changehomunculusname
	addpacket(0x023a, 4, NULL);
	addpacket(0x023b, 24, NULL,0); // storagepassword
	addpacket(0x023c, 6, NULL);

	//2005-06-22aSakexe
	addpacket(0x022e, 71, NULL);
#endif

	//2005-06-28aSakexe
#if CLIENTVER >= 17
	addpacket(0x0072, 34, NULL,6,17,30); // useskilltoid
	addpacket(0x007e, 113, NULL,12,15,18,31,33); // useskilltoposinfo
	addpacket(0x0085, 17, NULL,8,16); // changedir
	addpacket(0x0089, 13, packet_ticksend,9); // loadendack
	addpacket(0x008c, 8, NULL,4); // getcharnamerequest
	addpacket(0x0094, 31, NULL,16,27); // movetokafra
	addpacket(0x009b, 32, packet_wanttoconnect,9,15,23,27,31); // loadendackion
	addpacket(0x009f, 19, NULL,9,15); // useitem
	addpacket(0x00a2, 9, NULL,5); // solvecharname
	addpacket(0x00a7, 11, NULL,8); // walktoxy
	addpacket(0x00f5, 13, NULL,9); // takeitem
	addpacket(0x00f7, 18, NULL,11,14); // movefromkafra
	addpacket(0x0113, 33, NULL,12,15,18,31); // useskilltopos
	addpacket(0x0116, 12, NULL,3,10); // dropitem
	addpacket(0x0190, 24, NULL,11,23); // actionrequest
	addpacket(0x0216, -1, NULL);
	addpacket(0x023d, -1, NULL);
	addpacket(0x023e, 4, NULL);
#endif

	//2005-07-18aSakexe
#if CLIENTVER >= 18
	addpacket(0x0072, 19, NULL,5,11,15); // useskilltoid
	addpacket(0x007e, 110, NULL,9,15,23,28,30); // useskilltoposinfo
	addpacket(0x0085, 11, NULL,6,10); // changedir
	addpacket(0x0089, 7, packet_ticksend,3); // loadendack
	addpacket(0x008c, 11, NULL,7); // getcharnamerequest
	addpacket(0x0094, 21, NULL,12,17); // movetokafra
	addpacket(0x009b, 31, packet_wanttoconnect,3,13,22,26,30); // loadendackion
	addpacket(0x009f, 12, NULL,3,8); // useitem
	addpacket(0x00a2, 18, NULL,14); // solvecharname
	addpacket(0x00a7, 15, NULL,12); // walktoxy
	addpacket(0x00f5, 7, NULL,3); // takeitem
	addpacket(0x00f7, 13, NULL,5,9); // movefromkafra
	addpacket(0x0113, 30, NULL,9,15,23,28); // useskilltopos
	addpacket(0x0116, 12, NULL,6,10); // dropitem
	addpacket(0x0190, 21, NULL,5,20); // actionrequest
	addpacket(0x0216, 6, NULL);
	addpacket(0x023f, 2, NULL,0); // mailrefresh
	addpacket(0x0240, 8, NULL);
	addpacket(0x0241, 6, NULL,2); // mailread
	addpacket(0x0242, -1, NULL);
	addpacket(0x0243, 6, NULL,2); // maildelete
	addpacket(0x0244, 6, NULL,2); // mailgetattach
	addpacket(0x0245, 7, NULL);
	addpacket(0x0246, 4, NULL,2); // mailwinopen
	addpacket(0x0247, 8, NULL,2,4); // mailsetattach
	addpacket(0x0248, 68, NULL);
	addpacket(0x0249, 3, NULL);
	addpacket(0x024a, 70, NULL);
	addpacket(0x024b, 4, NULL,2); // auctioncancelreg
	addpacket(0x024c, 8, NULL,2,4); // auctionsetitem
	addpacket(0x024d, 14, NULL);
	addpacket(0x024e, 6, NULL,2); // auctioncancel
	addpacket(0x024f, 10, NULL,2,6); // auctionbid
	addpacket(0x0250, 3, NULL);
	addpacket(0x0251, 2, NULL);
	addpacket(0x0252, -1, NULL);
#endif

	//2005-07-19bSakexe
#if CLIENTVER >= 19
	addpacket(0x0072, 34, NULL,6,17,30); // useskilltoid
	addpacket(0x007e, 113, NULL,12,15,18,31,33); // useskilltoposinfo
	addpacket(0x0085, 17, NULL,8,16); // changedir
	addpacket(0x0089, 13, packet_ticksend,9); // loadendack
	addpacket(0x008c, 8, NULL,4); // getcharnamerequest
	addpacket(0x0094, 31, NULL,16,27); // movetokafra
	addpacket(0x009b, 32, packet_wanttoconnect,9,15,23,27,31); // loadendackion
	addpacket(0x009f, 19, NULL,9,15); // useitem
	addpacket(0x00a2, 9, NULL,5); // solvecharname
	addpacket(0x00a7, 11, NULL,8); // walktoxy
	addpacket(0x00f5, 13, NULL,9); // takeitem
	addpacket(0x00f7, 18, NULL,11,14); // movefromkafra
	addpacket(0x0113, 33, NULL,12,15,18,31); // useskilltopos
	addpacket(0x0116, 12, NULL,3,10); // dropitem
	addpacket(0x0190, 24, NULL,11,23); // actionrequest

	//2005-08-01aSakexe
	addpacket(0x0245, 3, NULL);
	addpacket(0x0251, 4, NULL);

	//2005-08-08aSakexe
	addpacket(0x024d, 12, NULL,2,6,10); // auctionregister
	addpacket(0x024e, 4, NULL);

	//2005-08-17aSakexe
	addpacket(0x0253, 3, NULL);
	addpacket(0x0254, 3, NULL,0); // feelsaveok

	//2005-08-29aSakexe
	addpacket(0x0240, -1, NULL);
	addpacket(0x0248, -1, NULL,2,4,28,68); // mailsend
	addpacket(0x0255, 5, NULL);
	addpacket(0x0256, -1, NULL);
	addpacket(0x0257, 8, NULL);

	//2005-09-12bSakexe
	addpacket(0x0256, 5, NULL);
	addpacket(0x0258, 2, NULL);
	addpacket(0x0259, 3, NULL);

	//2005-10-10aSakexe
	addpacket(0x020e, 32, NULL);
	addpacket(0x025a, -1, NULL);
	addpacket(0x025b, 6, NULL,0); // cooking

	//2005-10-13aSakexe
	addpacket(0x007a, 6, NULL);
	addpacket(0x0251, 32, NULL);
	addpacket(0x025c, 4, NULL,2); // auctionbuysell

	//2005-10-17aSakexe
	addpacket(0x007a, 58, NULL);
	addpacket(0x025d, 6, NULL,2); // auctionclose
	addpacket(0x025e, 4, NULL);

	//2005-10-24aSakexe
	addpacket(0x025f, 6, NULL);
	addpacket(0x0260, 6, NULL);

	//2005-11-07aSakexe
	addpacket(0x0251, 34, NULL,2,4,8,32); // auctionsearch

	//2006-01-09aSakexe
	addpacket(0x0261, 11, NULL);
	addpacket(0x0262, 11, NULL);
	addpacket(0x0263, 11, NULL);
	addpacket(0x0264, 20, NULL);
	addpacket(0x0265, 20, NULL);
	addpacket(0x0266, 30, NULL);
	addpacket(0x0267, 4, NULL);
	addpacket(0x0268, 4, NULL);
	addpacket(0x0269, 4, NULL);
	addpacket(0x026a, 4, NULL);
	addpacket(0x026b, 4, NULL);
	addpacket(0x026c, 4, NULL);
	addpacket(0x026d, 4, NULL);
	addpacket(0x026f, 2, NULL);
	addpacket(0x0270, 2, NULL);
	addpacket(0x0271, 38, NULL);
	addpacket(0x0272, 44, NULL);

	//2006-01-26aSakexe
	addpacket(0x0271, 40, NULL);

	//2006-03-06aSakexe
	addpacket(0x0273, 6, NULL);
	addpacket(0x0274, 8, NULL);

	//2006-03-13aSakexe
	addpacket(0x0273, 30, NULL,2,6); // mailreturn
#endif

	//2006-03-27aSakexe
#if CLIENTVER >= 20
	addpacket(0x0072, 26, NULL,11,18,22); // useskilltoid
	addpacket(0x007e, 120, NULL,5,15,29,38,40); // useskilltoposinfo
	addpacket(0x0085, 12, NULL,7,11); // changedir
	//addpacket(0x0089, 13, packet_ticksend,9); // loadendack
	addpacket(0x008c, 12, NULL,8); // getcharnamerequest
	addpacket(0x0094, 23, NULL,5,19); // movetokafra
	addpacket(0x009b, 37, packet_wanttoconnect,9,21,28,32,36); // loadendackion
	addpacket(0x009f, 24, NULL,9,20); // useitem
	addpacket(0x00a2, 11, NULL,7); // solvecharname
	addpacket(0x00a7, 15, NULL,12); // walktoxy
	addpacket(0x00f5, 13, NULL,9); // takeitem
	addpacket(0x00f7, 26, NULL,11,22); // movefromkafra
	addpacket(0x0113, 40, NULL,5,15,29,38); // useskilltopos
	addpacket(0x0116, 17, NULL,8,15); // dropitem
	addpacket(0x0190, 18, NULL,7,17); // actionrequest

	//2006-10-23aSakexe
	addpacket(0x006d, 110, NULL);

	//2006-04-24aSakexe to 2007-01-02aSakexe
	addpacket(0x023e, 8, NULL);
	addpacket(0x0277, 84, NULL);
	addpacket(0x0278, 2, NULL);
	addpacket(0x0279, 2, NULL);
	addpacket(0x027a, -1, NULL);
	addpacket(0x027b, 14, NULL);
	addpacket(0x027c, 60, NULL);
	addpacket(0x027d, 62, NULL);
	addpacket(0x027e, -1, NULL);
	addpacket(0x027f, 8, NULL);
	addpacket(0x0280, 12, NULL);
	addpacket(0x0281, 4, NULL);
	addpacket(0x0282, 284, NULL);
	addpacket(0x0283, 6, NULL);
	addpacket(0x0284, 14, NULL);
	addpacket(0x0285, 6, NULL);
	addpacket(0x0286, 4, NULL);
	addpacket(0x0287, -1, NULL);
	addpacket(0x0288, 6, NULL,2,4); // cashshopbuy
	addpacket(0x0289, 8, NULL);
	addpacket(0x028a, 18, NULL);
	addpacket(0x028b, -1, NULL);
	addpacket(0x028c, 46, NULL);
	addpacket(0x028d, 34, NULL);
	addpacket(0x028e, 4, NULL);
	addpacket(0x028f, 6, NULL);
	addpacket(0x0290, 4, NULL);
	addpacket(0x0291, 4, NULL);
	addpacket(0x0292, 2, NULL,0); // autorevive
	addpacket(0x0293, 70, NULL);
	addpacket(0x0294, 10, NULL);
	addpacket(0x0295, -1, NULL);
	addpacket(0x0296, -1, NULL);
	addpacket(0x0297, -1, NULL);
	addpacket(0x0298, 8, NULL);
	addpacket(0x0299, 6, NULL);
	addpacket(0x029a, 27, NULL);
	addpacket(0x029c, 66, NULL);
	addpacket(0x029d, -1, NULL);
	addpacket(0x029e, 11, NULL);
	addpacket(0x029f, 3, NULL,0); // mermenu
	addpacket(0x02a0, -1, NULL);
	addpacket(0x02a1, -1, NULL);
	addpacket(0x02a2, 8, NULL);
#endif

	//2007-01-08aSakexe
#if CLIENTVER >= 21
	addpacket(0x0072, 30, NULL,10,14,26); // useskilltoid
	addpacket(0x007e, 120, NULL,10,19,23,38,40); // useskilltoposinfo
	addpacket(0x0085, 14, NULL,10,13); // changedir
	addpacket(0x0089, 11, packet_ticksend,7); // loadendack
	addpacket(0x008c, 17, NULL,13); // getcharnamerequest
	addpacket(0x0094, 17, NULL,4,13); // movetokafra
	addpacket(0x009b, 35, packet_wanttoconnect,7,21,26,30,34); // loadendackion
	addpacket(0x009f, 21, NULL,7,17); // useitem
	addpacket(0x00a2, 10, NULL,6); // solvecharname
	addpacket(0x00a7, 8, NULL,5); // walktoxy
	addpacket(0x00f5, 11, NULL,7); // takeitem
	addpacket(0x00f7, 15, NULL,3,11); // movefromkafra
	addpacket(0x0113, 40, NULL,10,19,23,38); // useskilltopos
	addpacket(0x0116, 19, NULL,11,17); // dropitem
	addpacket(0x0190, 10, NULL,4,9); // actionrequest

	//2007-01-22aSakexe
	addpacket(0x02a3, 18, NULL);
	addpacket(0x02a4, 2, NULL);

	//2007-01-29aSakexe
	addpacket(0x029b, 72, NULL);
	addpacket(0x02a3, -1, NULL);
	addpacket(0x02a4, -1, NULL);
	addpacket(0x02a5, 8, NULL);

	// 2007-02-05aSakexe
	addpacket(0x02aa, 4, NULL);
	addpacket(0x02ab, 36, NULL);
	addpacket(0x02ac, 6, NULL);
#endif

	//2007-02-12aSakexe
#if CLIENTVER >= 22
	addpacket(0x0072, 25, NULL,6,10,21); // useskilltoid
	addpacket(0x007e, 102, NULL,5,9,12,20,22); // useskilltoposinfo
	addpacket(0x0085, 11, NULL,7,10); // changedir
	addpacket(0x0089, 8, packet_ticksend,4); // loadendack
	addpacket(0x008c, 11, NULL,7); // getcharnamerequest
	addpacket(0x0094, 14, NULL,7,10); // movetokafra
	addpacket(0x009b, 26, packet_wanttoconnect,4,9,17,21,25); // loadendackion
	addpacket(0x009f, 14, NULL,4,10); // useitem
	addpacket(0x00a2, 15, NULL,11); // solvecharname
	//addpacket(0x00a7, 8, NULL,5); // walktoxy
	addpacket(0x00f5, 8, NULL,4); // takeitem
	addpacket(0x00f7, 22, NULL,14,18); // movefromkafra
	addpacket(0x0113, 22, NULL,5,9,12,20); // useskilltopos
	addpacket(0x0116, 10, NULL,5,8); // dropitem
	addpacket(0x0190, 19, NULL,5,18); // actionrequest

	//2007-05-07aSakexe
	addpacket(0x01fd, 15, NULL,2); // repairitem

	//2007-02-27aSakexe to 2007-10-02aSakexe
	addpacket(0x0288, 10, NULL,2,4,6); // cashshopbuy
	addpacket(0x0289, 12, NULL);
	addpacket(0x02a6, 22, NULL);
	addpacket(0x02a7, 22, NULL);
	addpacket(0x02a8, 162, NULL);
	addpacket(0x02a9, 58, NULL);
	addpacket(0x02ad, 8, NULL);
	addpacket(0x02b0, 85, NULL);
	addpacket(0x02b1, -1, NULL);
	addpacket(0x02b2, -1, NULL);
	addpacket(0x02b3, 107, NULL);
	addpacket(0x02b4, 6, NULL);
	addpacket(0x02b5, -1, NULL);
	addpacket(0x02b6, 7, NULL,2,6); // queststate
	addpacket(0x02b7, 7, NULL);
	addpacket(0x02b8, 22, NULL);
	addpacket(0x02b9, 191, NULL);
	addpacket(0x02ba, 11, NULL,2,4,5,9); // hotkey
	addpacket(0x02bb, 8, NULL);
	addpacket(0x02bc, 6, NULL);
	addpacket(0x02bf, 10, NULL);
	addpacket(0x02c0, 2, NULL);
	addpacket(0x02c1, -1, NULL);
	addpacket(0x02c2, -1, NULL);
	addpacket(0x02c4, 26, NULL,2); // partyinvite2
	addpacket(0x02c5, 30, NULL);
	addpacket(0x02c6, 30, NULL);
	addpacket(0x02c7, 7, NULL,2,6); // replypartyinvite2
	addpacket(0x02c8, 3, NULL);
	addpacket(0x02c9, 3, NULL);
	addpacket(0x02ca, 3, NULL);
	addpacket(0x02cb, 20, NULL);
	addpacket(0x02cc, 4, NULL);
	addpacket(0x02cd, 26, NULL);
	addpacket(0x02ce, 10, NULL);
	addpacket(0x02cf, 6, NULL);
	addpacket(0x02d0, -1, NULL);
	addpacket(0x02d1, -1, NULL);
	addpacket(0x02d2, -1, NULL);
	addpacket(0x02d3, 4, NULL);
	addpacket(0x02d4, 29, NULL);
	addpacket(0x02d5, 2, NULL);
	addpacket(0x02d6, 6, NULL,2); // viewplayerequip
	addpacket(0x02d7, -1, NULL);
	addpacket(0x02d8, 10, NULL,6); // equiptickbox
	addpacket(0x02d9, 10, NULL);
	addpacket(0x02da, 3, NULL);
	addpacket(0x02db, -1, NULL,2,4); // battlechat
	addpacket(0x02dc, -1, NULL);
	addpacket(0x02dd, 32, NULL);
	addpacket(0x02de, 6, NULL);
	addpacket(0x02df, 36, NULL);
	addpacket(0x02e0, 34, NULL);

	//2007-10-23aSakexe
	addpacket(0x02cb, 65, NULL);
	addpacket(0x02cd, 71, NULL);

	//2007-11-06aSakexe
	addpacket(0x0078, 55, NULL);
	addpacket(0x007c, 42, NULL);
	addpacket(0x022c, 65, NULL);
	addpacket(0x029b, 80, NULL);

	//2007-11-13aSakexe
	addpacket(0x02e1, 33, NULL);

	//2007-11-20aSakexe
	//addpacket(0x01df, 10, NULL); <- ???
	addpacket(0x02e2, 14, NULL);
	addpacket(0x02e3, 25, NULL);
	addpacket(0x02e4, 8, NULL);
	addpacket(0x02e5, 8, NULL);
	addpacket(0x02e6, 6, NULL);

	//2007-11-27aSakexe
	addpacket(0x02e7, -1, NULL);

	//2008-01-02aSakexe
	addpacket(0x01df, 6, NULL,2); // gmreqaccname
	addpacket(0x02e8, -1, NULL);
	addpacket(0x02e9, -1, NULL);
	addpacket(0x02ea, -1, NULL);
	addpacket(0x02eb, 13, NULL);
	addpacket(0x02ec, 67, NULL);
	addpacket(0x02ed, 59, NULL);
	addpacket(0x02ee, 60, NULL);
	addpacket(0x02ef, 8, NULL);

	//2008-03-18aSakexe
	addpacket(0x02bf, -1, NULL);
	addpacket(0x02c0, -1, NULL);
	addpacket(0x02f0, 10, NULL);
	addpacket(0x02f1, 2, NULL,0); // progressbar
	addpacket(0x02f2, 2, NULL);

	//2008-03-25bSakexe
	addpacket(0x02f3, -1, NULL);
	addpacket(0x02f4, -1, NULL);
	addpacket(0x02f5, -1, NULL);
	addpacket(0x02f6, -1, NULL);
	addpacket(0x02f7, -1, NULL);
	addpacket(0x02f8, -1, NULL);
	addpacket(0x02f9, -1, NULL);
	addpacket(0x02fa, -1, NULL);
	addpacket(0x02fb, -1, NULL);
	addpacket(0x02fc, -1, NULL);
	addpacket(0x02fd, -1, NULL);
	addpacket(0x02fe, -1, NULL);
	addpacket(0x02ff, -1, NULL);
	addpacket(0x0300, -1, NULL);

	//2008-04-01aSakexe
	addpacket(0x0301, -1, NULL);
	addpacket(0x0302, -1, NULL);
	addpacket(0x0303, -1, NULL);
	addpacket(0x0304, -1, NULL);
	addpacket(0x0305, -1, NULL);
	addpacket(0x0306, -1, NULL);
	addpacket(0x0307, -1, NULL);
	addpacket(0x0308, -1, NULL);
	addpacket(0x0309, -1, NULL);
	addpacket(0x030a, -1, NULL);
	addpacket(0x030b, -1, NULL);
	addpacket(0x030c, -1, NULL);
	addpacket(0x030d, -1, NULL);
	addpacket(0x030e, -1, NULL);
	addpacket(0x030f, -1, NULL);
	addpacket(0x0310, -1, NULL);
	addpacket(0x0311, -1, NULL);
	addpacket(0x0312, -1, NULL);
	addpacket(0x0313, -1, NULL);
	addpacket(0x0314, -1, NULL);
	addpacket(0x0315, -1, NULL);
	addpacket(0x0316, -1, NULL);
	addpacket(0x0317, -1, NULL);
	addpacket(0x0318, -1, NULL);
	addpacket(0x0319, -1, NULL);
	addpacket(0x031a, -1, NULL);
	addpacket(0x031b, -1, NULL);
	addpacket(0x031c, -1, NULL);
	addpacket(0x031d, -1, NULL);
	addpacket(0x031e, -1, NULL);
	addpacket(0x031f, -1, NULL);
	addpacket(0x0320, -1, NULL);
	addpacket(0x0321, -1, NULL);
	addpacket(0x0322, -1, NULL);
	addpacket(0x0323, -1, NULL);
	addpacket(0x0324, -1, NULL);
	addpacket(0x0325, -1, NULL);
	addpacket(0x0326, -1, NULL);
	addpacket(0x0327, -1, NULL);
	addpacket(0x0328, -1, NULL);
	addpacket(0x0329, -1, NULL);
	addpacket(0x032a, -1, NULL);
	addpacket(0x032b, -1, NULL);
	addpacket(0x032c, -1, NULL);
	addpacket(0x032d, -1, NULL);
	addpacket(0x032e, -1, NULL);
	addpacket(0x032f, -1, NULL);
	addpacket(0x0330, -1, NULL);
	addpacket(0x0331, -1, NULL);
	addpacket(0x0332, -1, NULL);
	addpacket(0x0333, -1, NULL);
	addpacket(0x0334, -1, NULL);
	addpacket(0x0335, -1, NULL);
	addpacket(0x0336, -1, NULL);
	addpacket(0x0337, -1, NULL);
	addpacket(0x0338, -1, NULL);
	addpacket(0x0339, -1, NULL);
	addpacket(0x033a, -1, NULL);
	addpacket(0x033b, -1, NULL);
	addpacket(0x033c, -1, NULL);
	addpacket(0x033d, -1, NULL);
	addpacket(0x033e, -1, NULL);
	addpacket(0x033f, -1, NULL);
	addpacket(0x0340, -1, NULL);
	addpacket(0x0341, -1, NULL);
	addpacket(0x0342, -1, NULL);
	addpacket(0x0343, -1, NULL);
	addpacket(0x0344, -1, NULL);
	addpacket(0x0345, -1, NULL);
	addpacket(0x0346, -1, NULL);
	addpacket(0x0347, -1, NULL);
	addpacket(0x0348, -1, NULL);
	addpacket(0x0349, -1, NULL);
	addpacket(0x034a, -1, NULL);
	addpacket(0x034b, -1, NULL);
	addpacket(0x034c, -1, NULL);
	addpacket(0x034d, -1, NULL);
	addpacket(0x034e, -1, NULL);
	addpacket(0x034f, -1, NULL);
	addpacket(0x0350, -1, NULL);
	addpacket(0x0351, -1, NULL);
	addpacket(0x0352, -1, NULL);
	addpacket(0x0353, -1, NULL);
	addpacket(0x0354, -1, NULL);
	addpacket(0x0355, -1, NULL);
	addpacket(0x0356, -1, NULL);
	addpacket(0x0357, -1, NULL);
	addpacket(0x0358, -1, NULL);
	addpacket(0x0359, -1, NULL);
	addpacket(0x035a, -1, NULL);

	//2008-05-27aSakexe
	addpacket(0x035b, -1, NULL);
	addpacket(0x035c, 2, NULL);
	addpacket(0x035d, -1, NULL);
	addpacket(0x035e, 2, NULL);
	addpacket(0x035f, -1, NULL);
	addpacket(0x0389, -1, NULL);

	//2008-08-20aSakexe
	addpacket(0x040c, -1, NULL);
	addpacket(0x040d, -1, NULL);
	addpacket(0x040e, -1, NULL);
	addpacket(0x040f, -1, NULL);
	addpacket(0x0410, -1, NULL);
	addpacket(0x0411, -1, NULL);
	addpacket(0x0412, -1, NULL);
	addpacket(0x0413, -1, NULL);
	addpacket(0x0414, -1, NULL);
	addpacket(0x0415, -1, NULL);
	addpacket(0x0416, -1, NULL);
	addpacket(0x0417, -1, NULL);
	addpacket(0x0418, -1, NULL);
	addpacket(0x0419, -1, NULL);
	addpacket(0x041a, -1, NULL);
	addpacket(0x041b, -1, NULL);
	addpacket(0x041c, -1, NULL);
	addpacket(0x041d, -1, NULL);
	addpacket(0x041e, -1, NULL);
	addpacket(0x041f, -1, NULL);
	addpacket(0x0420, -1, NULL);
	addpacket(0x0421, -1, NULL);
	addpacket(0x0422, -1, NULL);
	addpacket(0x0423, -1, NULL);
	addpacket(0x0424, -1, NULL);
	addpacket(0x0425, -1, NULL);
	addpacket(0x0426, -1, NULL);
	addpacket(0x0427, -1, NULL);
	addpacket(0x0428, -1, NULL);
	addpacket(0x0429, -1, NULL);
	addpacket(0x042a, -1, NULL);
	addpacket(0x042b, -1, NULL);
	addpacket(0x042c, -1, NULL);
	addpacket(0x042d, -1, NULL);
	addpacket(0x042e, -1, NULL);
	addpacket(0x042f, -1, NULL);
	addpacket(0x0430, -1, NULL);
	addpacket(0x0431, -1, NULL);
	addpacket(0x0432, -1, NULL);
	addpacket(0x0433, -1, NULL);
	addpacket(0x0434, -1, NULL);
	addpacket(0x0435, -1, NULL);
#endif

	//2008-09-10aSakexe
#if CLIENTVER >= 23
	addpacket(0x0436, 19, packet_wanttoconnect,2,6,10,14,18); // loadendackion
	addpacket(0x0437, 7, NULL,2,6); // actionrequest
	addpacket(0x0438, 10, NULL,2,4,6); // useskilltoid
	addpacket(0x0439, 8, NULL,2,4); // useitem

	//2008-11-13aSakexe
	addpacket(0x043d, 8, NULL);
	addpacket(0x043e, -1, NULL);
	addpacket(0x043f, 8, NULL);

	//2008-11-26aSakexe
	addpacket(0x01a2, 37, NULL);
	addpacket(0x0440, 10, NULL);
	addpacket(0x0441, 4, NULL);

	//2008-12-10aSakexe
	addpacket(0x0442, -1, NULL);
	addpacket(0x0443, 8, NULL,2,6); // skillselectmenu

	//2009-01-14aSakexe
	addpacket(0x043f, 25, NULL);
	addpacket(0x0444, -1, NULL);
	addpacket(0x0445, 10, NULL);

	//2009-02-18aSakexe
	addpacket(0x0446, 14, NULL);

	//2009-02-25aSakexe
	addpacket(0x0448, -1, NULL);

	//2009-03-30aSakexe
	addpacket(0x0449, 4, NULL);

	//2009-04-08aSakexe
	addpacket(0x02a6, -1, NULL);
	addpacket(0x02a7, -1, NULL);
	addpacket(0x044a, 6, NULL);
#endif

	//Renewal Clients
	//2008-08-27aRagexeRE
#if CLIENTVER >= 24
	addpacket(0x0072, 22, NULL,9,15,18); // useskilltoid
	addpacket(0x007c, 44, NULL);
	addpacket(0x007e, 105, NULL,10,14,18,23,25); // useskilltoposinfo
	addpacket(0x0085, 10, NULL,4,9); // changedir
	addpacket(0x0089, 11, packet_ticksend,7); // loadendack
	addpacket(0x008c, 14, NULL,10); // getcharnamerequest
	addpacket(0x0094, 19, NULL,3,15); // movetokafra
	addpacket(0x009b, 34, packet_wanttoconnect,7,15,25,29,33); // loadendackion
	addpacket(0x009f, 20, NULL,7,20); // useitem
	addpacket(0x00a2, 14, NULL,10); // solvecharname
	addpacket(0x00a7, 9, NULL,6); // walktoxy
	addpacket(0x00f5, 11, NULL,7); // takeitem
	addpacket(0x00f7, 17, NULL,3,13); // movefromkafra
	addpacket(0x0113, 25, NULL,10,14,18,23); // useskilltopos
	addpacket(0x0116, 17, NULL,6,15); // dropitem
	addpacket(0x0190, 23, NULL,9,22); // actionrequest
	addpacket(0x02e2, 20, NULL);
	addpacket(0x02e3, 22, NULL);
	addpacket(0x02e4, 11, NULL);
	addpacket(0x02e5, 9, NULL);
#endif

	//2008-09-10aRagexeRE
#if CLIENTVER >= 25
	addpacket(0x0436, 19, packet_wanttoconnect,2,6,10,14,18); // loadendackion
	addpacket(0x0437, 7, NULL,2,6); // actionrequest
	addpacket(0x0438, 10, NULL,2,4,6); // useskilltoid
	addpacket(0x0439, 8, NULL,2,4); // useitem

	//2008-11-12aRagexeRE
	addpacket(0x043d, 8, NULL);
	//addpacket(0x043e, -1, NULL);
	addpacket(0x043f, 8, NULL);

	//2008-12-17aRagexeRE
	addpacket(0x01a2, 37, NULL);
	//addpacket(0x0440, 10, NULL);
	//addpacket(0x0441, 4, NULL);
	//addpacket(0x0442, 8, NULL);
	//addpacket(0x0443, 8, NULL);

	//2008-12-17bRagexeRE
	addpacket(0x006d, 114, NULL);

	//2009-01-21aRagexeRE
	addpacket(0x043f, 25, NULL);
	//addpacket(0x0444, -1, NULL);
	//addpacket(0x0445, 10, NULL);

	//2009-02-18aRagexeRE
	//addpacket(0x0446, 14, NULL);

	//2009-02-26cRagexeRE
	//addpacket(0x0448, -1, NULL);

	//2009-04-01aRagexeRE
	//addpacket(0x0449, 4, NULL);

	//2009-05-14aRagexeRE
	//addpacket(0x044b, 2, NULL);

	//2009-05-20aRagexeRE
	//addpacket(0x07d0, 6, NULL);
	//addpacket(0x07d1, 2, NULL);
	//addpacket(0x07d2, -1, NULL);
	//addpacket(0x07d3, 4, NULL);
	//addpacket(0x07d4, 4, NULL);
	//addpacket(0x07d5, 4, NULL);
	//addpacket(0x07d6, 4, NULL);
	//addpacket(0x0447, 2, NULL);

	//2009-06-03aRagexeRE
	addpacket(0x07d7, 8, NULL,2,6); // partychangeoption
	addpacket(0x07d8, 8, NULL);
	addpacket(0x07d9, 254, NULL);
	addpacket(0x07da, 6, NULL,2); // partychangeleader

	//2009-06-10aRagexeRE
	//addpacket(0x07db, 8, NULL);

	//2009-06-17aRagexeRE
	addpacket(0x07d9, 268, NULL);
	//addpacket(0x07dc, 6, NULL);
	//addpacket(0x07dd, 54, NULL);
	//addpacket(0x07de, 30, NULL);
	//addpacket(0x07df, 54, NULL);

	//2009-07-01aRagexeRE
	//addpacket(0x0275, 37, NULL);
	//addpacket(0x0276, -1, NULL);

	//2009-07-08aRagexeRE
	//addpacket(0x07e0, 58, NULL);

	//2009-07-15aRagexeRE
	addpacket(0x07e1, 15, NULL);

	//2009-08-05aRagexeRE
	addpacket(0x07e2, 8, NULL);

	//2009-08-18aRagexeRE
	addpacket(0x07e3, 6, NULL);
	addpacket(0x07e4, -1, NULL,2,4,8); // itemlistwindowselected
	addpacket(0x07e6, 8, NULL);

	//2009-08-25aRagexeRE
	//addpacket(0x07e6, 28, NULL);
	addpacket(0x07e7, 5, NULL);

	//2009-09-22aRagexeRE
	addpacket(0x07e5, 8, NULL);
	addpacket(0x07e6, 8, NULL);
	addpacket(0x07e7, 32, NULL);
	addpacket(0x07e8, -1, NULL);
	addpacket(0x07e9, 5, NULL);

	//2009-09-29aRagexeRE
	//addpacket(0x07ea, 2, NULL);
	//addpacket(0x07eb, -1, NULL);
	//addpacket(0x07ec, 6, NULL);
	//addpacket(0x07ed, 8, NULL);
	//addpacket(0x07ee, 6, NULL);
	//addpacket(0x07ef, 8, NULL);
	//addpacket(0x07f0, 4, NULL);
	//addpacket(0x07f2, 4, NULL);
	//addpacket(0x07f3, 3, NULL);

	//2009-10-06aRagexeRE
	//addpacket(0x07ec, 8, NULL);
	//addpacket(0x07ed, 10, NULL);
	//addpacket(0x07f0, 8, NULL);
	//addpacket(0x07f1, 15, NULL);
	//addpacket(0x07f2, 6, NULL);
	//addpacket(0x07f3, 4, NULL);
	//addpacket(0x07f4, 3, NULL);

	//2009-10-27aRagexeRE
	addpacket(0x07f5, 6, NULL,2); // gmreqaccname
	addpacket(0x07f6, 14, NULL);

	//2009-11-03aRagexeRE
	addpacket(0x07f7, -1, NULL);
	addpacket(0x07f8, -1, NULL);
	addpacket(0x07f9, -1, NULL);

	//2009-11-17aRagexeRE
	addpacket(0x07fa, 8, NULL);

	//2009-11-24aRagexeRE
	addpacket(0x07fb, 25, NULL);

	//2009-12-01aRagexeRE
	//addpacket(0x07fc, 10, NULL);
	//addpacket(0x07fd, -1, NULL);
	addpacket(0x07fe, 26, NULL);
	//addpacket(0x07ff, -1, NULL);

	//2009-12-15aRagexeRE
	//addpacket(0x0800, -1, NULL);
	//addpacket(0x0801, -1, NULL);

	//2009-12-22aRagexeRE
	addpacket(0x0802, 18, NULL,2,4,6); // bookingregreq
	addpacket(0x0803, 4, NULL);
	addpacket(0x0804, 8, NULL);		// Booking System
	addpacket(0x0805, -1, NULL);
	addpacket(0x0806, 4, NULL,2); // bookingdelreq
	//addpacket(0x0807, 2, NULL);
	addpacket(0x0808, 4, NULL);		// Booking System
	//addpacket(0x0809, 14, NULL);
	//addpacket(0x080A, 50, NULL);
	//addpacket(0x080B, 18, NULL);
	//addpacket(0x080C, 6, NULL);

	//2009-12-29aRagexeRE
	addpacket(0x0804, 14, NULL,2,4,6,8,12); // bookingsearchreq
	addpacket(0x0806, 2, NULL,0); // bookingdelreq
	addpacket(0x0807, 4, NULL);
	addpacket(0x0808, 14, NULL,2); // bookingupdatereq
	addpacket(0x0809, 50, NULL);
	addpacket(0x080A, 18, NULL);
	addpacket(0x080B, 6, NULL);		// Booking System

	//2010-01-05aRagexeRE
	addpacket(0x0801, -1, NULL,2,4,8,12); // purchasereq2

	//2010-01-26aRagexeRE
	//addpacket(0x080C, 2, NULL);
	//addpacket(0x080D, 3, NULL);
	addpacket(0x080E, 14, NULL);

	//2010-02-09aRagexeRE
	//addpacket(0x07F0, 6, NULL);

	//2010-02-23aRagexeRE
	addpacket(0x080F, 20, NULL);

	//2010-03-03aRagexeRE
	addpacket(0x0810, 3, NULL);
	addpacket(0x0811, -1, NULL,2,4,8,9,89); // reqopenbuyingstore
	//addpacket(0x0812, 86, NULL);
	//addpacket(0x0813, 6, NULL);
	//addpacket(0x0814, 6, NULL);
	//addpacket(0x0815, -1, NULL);
	//addpacket(0x0817, -1, NULL);
	//addpacket(0x0818, 6, NULL);
	//addpacket(0x0819, 4, NULL);

	//2010-03-09aRagexeRE
	addpacket(0x0813, -1, NULL);
	//addpacket(0x0814, 2, NULL);
	//addpacket(0x0815, 6, NULL);
	addpacket(0x0816, 6, NULL);
	addpacket(0x0818, -1, NULL);
	//addpacket(0x0819, 10, NULL);
	//addpacket(0x081A, 4, NULL);
	//addpacket(0x081B, 4, NULL);
	//addpacket(0x081C, 6, NULL);
	addpacket(0x081d, 22, NULL);
	addpacket(0x081e, 8, NULL);

	//2010-03-23aRagexeRE
	//addpacket(0x081F, -1, NULL);

	//2010-04-06aRagexeRE
	//addpacket(0x081A, 6, NULL);

	//2010-04-13aRagexeRE
	//addpacket(0x081A, 10, NULL);
	addpacket(0x0820, 11, NULL);
	//addpacket(0x0821, 2, NULL);
	//addpacket(0x0822, 9, NULL);
	//addpacket(0x0823, -1, NULL);

	//2010-04-14dRagexeRE
	//addpacket(0x081B, 8, NULL);

	//2010-04-20aRagexeRE
	addpacket(0x0812, 8, NULL);
	addpacket(0x0814, 86, NULL);
	addpacket(0x0815, 2, NULL,0); // reqclosebuyingstore
	addpacket(0x0817, 6, NULL,2); // reqclickbuyingstore
	addpacket(0x0819, -1, NULL,2,4,8,12); // reqtradebuyingstore
	addpacket(0x081a, 4, NULL);
	addpacket(0x081b, 10, NULL);
	addpacket(0x081c, 10, NULL);
	addpacket(0x0824, 6, NULL);

	//2010-06-01aRagexeRE
	//addpacket(0x0825, -1, NULL);
	//addpacket(0x0826, 4, NULL);
	addpacket(0x0835, -1, NULL,2,4,5,9,13,14,15); // searchstoreinfo
	addpacket(0x0836, -1, NULL);
	addpacket(0x0837, 3, NULL);
	//addpacket(0x0838, 3, NULL);

	//2010-06-08aRagexeRE
	addpacket(0x0838, 2, NULL,0); // searchstoreinfonextpage
	addpacket(0x083A, 4, NULL); 	// Search Stalls Feature
	addpacket(0x083B, 2, NULL,0); // closesearchstoreinfo
	addpacket(0x083C, 12, NULL,2,6,10); // searchstoreinfolistitemclick
	addpacket(0x083D, 6, NULL);

	//2010-06-15aRagexeRE
	//addpacket(0x083E, 26, NULL);

	//2010-06-22aRagexeRE
	//addpacket(0x083F, 22, NULL);

	//2010-06-29aRagexeRE
	addpacket(0x00AA, 9, NULL);
	//addpacket(0x07F1, 18, NULL);
	//addpacket(0x07F2, 8, NULL);
	//addpacket(0x07F3, 6, NULL);

	//2010-07-01aRagexeRE
	addpacket(0x083A, 5, NULL); 	// Search Stalls Feature

	//2010-07-13aRagexeRE
	//addpacket(0x0827, 6, NULL);
	//addpacket(0x0828, 14, NULL);
	//addpacket(0x0829, 6, NULL);
	//addpacket(0x082A, 10, NULL);
	//addpacket(0x082B, 6, NULL);
	//addpacket(0x082C, 14, NULL);
	//addpacket(0x0840, -1, NULL);
	//addpacket(0x0841, 19, NULL);

	//2010-07-14aRagexeRE
	//addpacket(0x841, 4, NULL);

	//2010-08-03aRagexeRE
	addpacket(0x0839, 66, NULL);
	addpacket(0x0842, 6, NULL,2); // recall2
	addpacket(0x0843, 6, NULL,2); // remove2
#endif

	//2010-11-24aRagexeRE
#if CLIENTVER >= 26
	addpacket(0x0288, -1, NULL,4,8); // cashshopbuy
	addpacket(0x0436, 19, packet_wanttoconnect,2,6,10,14,18); // loadendackion
	addpacket(0x035f, 5, NULL,2); // walktoxy
	addpacket(0x0360, 6, packet_ticksend,2); // loadendack
	addpacket(0x0361, 5, NULL,2,4); // changedir
	addpacket(0x0362, 6, NULL,2); // takeitem
	addpacket(0x0363, 6, NULL,2,4); // dropitem
	addpacket(0x0364, 8, NULL,2,4); // movetokafra
	addpacket(0x0365, 8, NULL,2,4); // movefromkafra
	addpacket(0x0366, 10, NULL,2,4,6,8); // useskilltopos
	addpacket(0x0367, 90, NULL,2,4,6,8,10); // useskilltoposinfo
	addpacket(0x0368, 6, NULL,2); // getcharnamerequest
	addpacket(0x0369, 6, NULL,2); // solvecharname
	addpacket(0x0856, -1, NULL);
	addpacket(0x0857, -1, NULL);
	addpacket(0x0858, -1, NULL);
	addpacket(0x0859, -1, NULL);
#endif
}

int packet_msgsend(const unsigned char* buf, int len, struct block_list* bl, TalkArea type)
{
	int i;

	switch(type) {

	case COMMONTALK_AREA:
		{

		}
		break;

	}

	return 0;
}

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

	*_name = NULL; *_mes = NULL;

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

	if (type == 0)
	{

		name = text;
		namelen = strnlen(zd->status.name , NAME_LENGTH-1);

		if( strncmp(name, zd->status.name , namelen) || name[namelen] != ' ' || name[namelen+1] != ':' || name[namelen+2] != ' ' ) 
		{
			ShowWarning("packethandle_msgparse: Client Desync bug received from '%s'\n", zd->status.name );
			return false;
		}

		mes = name + namelen + 3;
		meslen = textlen - namelen - 3;

	}
	else
	{ 

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

	name[namelen] = 0;

	return true;
}

/*! 
*  \brief     Chat Packets Manipulation
*  \details   General player talk messages.
*  \author    Fimbulwinter Development Team
*  \author    Castor, GreenBox
*  \date      29/12/11
*
**/
void packet_chatpackets(tcp_connection::pointer cl, struct ZoneSessionData* zd, enum typechat tc)
{
	const char* text = (char*)RFIFOP(cl,4);
	int textlen = RFIFOW(cl,2) - 4;

	char *name, *message;
	int namelen, messagelen;

	if( !packet_msgformat(zd, &name, &message, 0) )
		return;

	

	switch(tc)
	{
	}
}

/*! 
*  \brief     Packet Want to Connect
*  \details   Request a connection to the Zone Server
*  \author    Fimbulwinter Development Team
*  \author    GreenBox
*  \date      ??/12/11
*
**/
void packet_wanttoconnect(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	int cmd, account_id, char_id, login_id1, sex;
	unsigned int client_tick;

	cmd = RFIFOW(cl,0);
	account_id  = RFIFOL(cl, ZoneServer::client_packets[cmd]->pos[0]);
	char_id     = RFIFOL(cl, ZoneServer::client_packets[cmd]->pos[1]);
	login_id1   = RFIFOL(cl, ZoneServer::client_packets[cmd]->pos[2]);
	client_tick = RFIFOL(cl, ZoneServer::client_packets[cmd]->pos[3]);
	sex         = RFIFOB(cl, ZoneServer::client_packets[cmd]->pos[4]);

	struct BlockList *bl = BlockManager::get_block(account_id);

	if (bl && bl->type != BL_PC)
	{
		ShowError("packet_wanttoconnect: a non-player object already has id %d, please increase the starting account number.\n", account_id);

		WFIFOPACKET(cl,packet,AC_REFUSE_LOGIN);
		packet->header = HEADER_AC_REFUSE_LOGIN;
		packet->error_code = 3; // Rejected By Server
		cl->send_buffer(sizeof(struct PACKET_AC_REFUSE_LOGIN));
		cl->set_eof();

		return;
	}

	if (bl || ZoneServer::auth_nodes.count(account_id))
	{
		ZoneServer::auth_fail(cl, 8);
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
	WFIFOPACKET(cl,packet,ZC_AID);
	packet->header = HEADER_ZC_AID;
	packet->AID = sd->bl.id;
    cl->send_buffer(6);
#endif

	ZoneServer::inter_confirm_auth(sd);
}

/*! 
*  \brief     Less Effect Packet
*  \details   "/effect"
*  \author    Fimbulwinter Development Team
*  \author    GreenBox
*  \date      ??/12/11
*
**/
void packet_lesseffect(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	int isLess = RFIFOL(cl, ZoneServer::client_packets[RFIFOW(cl, 0)]->pos[0]);

	sd->state.lesseffect = (isLess != 0);
}

/*! 
*  \brief     Load Enviroment
*  \details   Load the Enviroment informations
*  \author    Fimbulwinter Development Team
*  \author    GreenBox
*  \date      ??/12/11
*
**/
void packet_loadendack(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	if(sd->bl.prev != NULL)
		return;

	if (!sd->state.active)
	{
		sd->state.connect_new = 0;

		return;
	}

	// TODO: Update Look, Items, Cart, Guild, Party and Guild
	ZoneServer::addblock(&sd->bl);
	ZoneServer::clif_spawn(&sd->bl);

	// TODO: Send Map Properties
	// TODO: Info about nearby objects
}

/*! 
*  \brief     Clock Tick Send
*  \details   Send Time
*  \author    Fimbulwinter Development Team
*  \author    GreenBox
*  \date      ??/12/11
*
**/
void packet_ticksend(tcp_connection::pointer cl, ZoneSessionData *sd)
{
	sd->client_tick = RFIFOL(cl, ZoneServer::client_packets[RFIFOW(cl, 0)]->pos[0]);

	WFIFOPACKET(cl,packet,ZC_NOTIFY_TIME);
	packet->header = HEADER_ZC_NOTIFY_TIME;
	packet->time = (unsigned int)time(NULL);
	cl->send_buffer(sizeof(struct PACKET_ZC_NOTIFY_TIME));
}
