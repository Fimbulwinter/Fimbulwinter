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
*                       Client to Auth Modules 	               	    *
* ==================================================================*/

#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <packets.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>

#include <md5.hpp>
#include <strfuncs.hpp>
#include <boost/foreach.hpp>


/*==============================================================*
* Function:	Parse from Client (Auth)							*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Parse informations from the client				*
**==============================================================*/
int AuthServer::parse_from_client(tcp_connection::pointer cl)
{
	AuthSessionData *asd = ((AuthSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		if (asd)
			delete asd;

		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	if (asd == NULL)
	{
		asd = new AuthSessionData();
		memset(asd, 0, sizeof(asd));

		asd->cl = cl;

		cl->set_data((char*)asd);
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch(cmd)
		{
			// request client login (raw password)
		case HEADER_CA_LOGIN:			// S 0064 <version>.L <username>.24B <password>.24B <clienttype>.B
		case HEADER_CA_LOGIN_PCBANG:	// S 0277 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B
		case HEADER_CA_LOGIN_CHANNEL:	// S 02b0 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B <g_isGravityID>.B
			// request client login (md5-hashed password)
		case HEADER_CA_LOGIN2:			// S 01dd <version>.L <username>.24B <password hash>.16B <clienttype>.B
		case HEADER_CA_LOGIN3:			// S 01fa <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.B(index of the connection in the clientinfo file (+10 if the command-line contains "pc"))
		case HEADER_CA_LOGIN4:			// S 027c <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.13B(junk)
			// token-based login
		case HEADER_CA_LOGIN_TOKEN:		// S 0825 <packetsize>.W <version>.L <clienttype>.B <userid>.24B <password>.27B <mac>.17B <ip>.15B <token>.(packetsize - 0x5C)B
			{
				size_t packet_len = RFIFOREST(cl);
				unsigned int version;
				char username[NAME_LENGTH];
				char password[NAME_LENGTH];
				unsigned char passhash[16];
				unsigned char clienttype;
				bool israwpass = (cmd == HEADER_CA_LOGIN || cmd == HEADER_CA_LOGIN_PCBANG || cmd == HEADER_CA_LOGIN_CHANNEL || cmd == HEADER_CA_LOGIN_TOKEN);

				if((cmd == HEADER_CA_LOGIN && packet_len < sizeof(struct PACKET_CA_LOGIN))
					||  (cmd == HEADER_CA_LOGIN_PCBANG && packet_len < sizeof(struct PACKET_CA_LOGIN_PCBANG))
					||  (cmd == HEADER_CA_LOGIN_CHANNEL && packet_len < sizeof(struct PACKET_CA_LOGIN_CHANNEL))
					||  (cmd == HEADER_CA_LOGIN2 && packet_len < sizeof(struct PACKET_CA_LOGIN2))
					||  (cmd == HEADER_CA_LOGIN3 && packet_len < sizeof(struct PACKET_CA_LOGIN3))
					||  (cmd == HEADER_CA_LOGIN4 && packet_len < sizeof(struct PACKET_CA_LOGIN4)) 
					||  (cmd == HEADER_CA_LOGIN_TOKEN && (packet_len < 4 || packet_len < RFIFOW(cl, 2)))
					)
					return 0;

				// Token-based authentication model by Shinryo
				if(cmd == HEADER_CA_LOGIN_TOKEN)
				{	
					char *accname = (char *)RFIFOP(cl, 9);
					char *token = (char *)RFIFOP(cl, 0x5C);
					size_t uAccLen = strlen(accname);
					size_t uTokenLen = packet_len - 0x5C;

					version = RFIFOL(cl,4);

					if(uAccLen > NAME_LENGTH - 1 || uAccLen <= 0 || uTokenLen != 32)
					{
						// TODO: Failed
						return 0;
					}

					clienttype = RFIFOB(cl, 8);
				}
				else
				{
					version = RFIFOL(cl,2);
					strncpy(username, (const char*)RFIFOP(cl,6), NAME_LENGTH);

					if( israwpass )
					{
						strncpy(password, (const char*)RFIFOP(cl,30), NAME_LENGTH);
						clienttype = RFIFOB(cl,54);
					}
					else
					{
						memcpy(passhash, RFIFOP(cl,30), 16);
						clienttype = RFIFOB(cl,46);
					}
				}			

				cl->skip(RFIFOREST(cl)); // assume no other packet was sent

				asd->clienttype = clienttype;
				asd->version = version;
				strncpy(asd->username, username, NAME_LENGTH);

				ShowStatus("Request for connection of %s from %s.\n", asd->username, cl->socket().remote_endpoint().address().to_string().c_str());
				if (cmd == HEADER_CA_LOGIN_TOKEN)
				{
					// TODO: Add support to token-based login

					asd->type = auth_token;
				}
				else if (israwpass)
				{
					strncpy(asd->password, password, NAME_LENGTH);

					if(config.auth_use_md5)
						md5(asd->password);

					asd->type = auth_raw;
				}
				else
				{
					bin2hex(asd->password, passhash, 16); // raw binary data here!
					asd->type = auth_md5;
				}

				int result = authenticate(asd);

				if (result != -1)
					send_auth_err(asd, result);
				else
					send_auth_ok(asd);
			}
			break;

		// MD5 Login
		case HEADER_CA_REQ_HASH:
			{
				memset(asd->md5key, '\0', sizeof(asd->md5key));
				asd->md5keylen = (boost::uint16_t)(12 + rand() % 4);
				MD5_Salt(asd->md5keylen, asd->md5key);

				WFIFOHEAD(cl, sizeof(PACKET_AC_ACK_HASH) + asd->md5keylen);
				TYPECAST_PACKET(WFIFOP(cl, 0), spacket, AC_ACK_HASH);

				spacket->header = HEADER_AC_ACK_HASH;
				spacket->packet_len = sizeof(PACKET_AC_ACK_HASH) + asd->md5keylen;
				memcpy(spacket->salt, asd->md5key, asd->md5keylen);

				cl->send_buffer(spacket->packet_len);
				cl->skip(sizeof(PACKET_CA_REQ_HASH));
			}
			break;

		// nProtect GameGuard Challenge
		case HEADER_CA_REQ_GAME_GUARD_CHECK:
			{
				WFIFOPACKET(cl, spacket, AC_ACK_GAME_GUARD);

				spacket->answer = ((asd->gameguardChallenged)?(2):(1));
				asd->gameguardChallenged = true;

				cl->send_buffer(sizeof(struct PACKET_AC_ACK_GAME_GUARD));
				cl->skip(sizeof(PACKET_CA_REQ_GAME_GUARD_CHECK));
			}
			break;

		// Inter Server to Char Server login
		case INTER_CA_LOGIN: // S 3000 <login>.24B <password>.24B <display name>.20B
			if (RFIFOREST(cl) < 76)
				return 0;
			{
				char server_name[20];
				address_v4 addr;
				unsigned short port;

				strncpy(asd->username, (char*)RFIFOP(cl, 2), NAME_LENGTH);
				strncpy(asd->password, (char*)RFIFOP(cl, 26), NAME_LENGTH);
				strncpy(server_name, (char*)RFIFOP(cl, 50), 20);

				addr = address_v4(ntohl(RFIFOL(cl, 70)));
				port = ntohs(RFIFOW(cl, 74));

				cl->skip(76);

				asd->type = auth_raw;

				int result = authenticate(asd);
				if (result == -1 && asd->sex == 'S' && !servers.count(asd->account_id))
				{
					servers[asd->account_id].addr = addr;
					servers[asd->account_id].port = port;
					servers[asd->account_id].account_id = asd->account_id;
					servers[asd->account_id].cl = cl;

					strncpy(servers[asd->account_id].name, server_name, 20);

					cl->flags.server = 1;
					cl->realloc_fifo(FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
					cl->set_parser(&AuthServer::parse_from_char);

					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = INTER_AC_LOGIN_REPLY;
					WFIFOB(cl,2) = 0;
					cl->send_buffer(3);
				}
				else
				{
					ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);

					WFIFOHEAD(cl,3);
					WFIFOW(cl,0) = INTER_AC_LOGIN_REPLY;
					WFIFOB(cl,2) = 3;
					cl->send_buffer(3);
				}
			}
			break;

		default:
			ShowWarning("Unknown packet 0x%04x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}
