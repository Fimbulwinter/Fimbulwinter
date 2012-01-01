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

#include "CharServer.hpp"

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

/*! 
 *  \brief     Parse from Client
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
int CharServer::parse_from_client(tcp_connection::pointer cl)
{
	CharSessionData *csd = ((CharSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		if (csd && csd->auth && auth_conn_ok)
		{
			WFIFOHEAD(auth_conn,6);
			WFIFOW(auth_conn,0) = INTER_CA_SET_ACC_OFF;
			WFIFOL(auth_conn,2) = csd->account_id;
			auth_conn->send_buffer(6);
		}

		set_char_offline(csd->account_id, -1);

		if (csd)
			delete csd;

		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

#define FIFOSD_CHECK(rest) { if(RFIFOREST(cl) < rest) return 0; if (csd==NULL || !csd->auth) { cl->skip(rest); return 0; } }

		switch (cmd)
		{
		case HEADER_CH_SELECT_CHAR:
			FIFOSD_CHECK(3);
			{
				int slot = RFIFOB(cl,2);
				int char_id;
				CharData cd;

				cl->skip(3);

				{
					statement s = (database->prepare << "SELECT `char_id` FROM `char` WHERE `account_id`=:a AND `char_num`=:s",
						use(csd->account_id), use(slot), into(char_id));

					s.execute(true);

					if (s.get_affected_rows() <= 0)
					{
						WFIFOPACKET(cl, spacket, HC_REFUSE_ENTER);
						spacket->error_code = 0;
						cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_ENTER));
					}
				}

				chars->load_char(char_id, cd, true);

				int server = -1;
				if (map_to_zone.count(cd.last_point.map))
					server = map_to_zone[cd.last_point.map];

				if (server < 0)
				{
					// TODO: Find for major city

					WFIFOPACKET(cl, spacket, SC_NOTIFY_BAN);
					spacket->error_code = 1;
					cl->send_buffer(sizeof(struct PACKET_SC_NOTIFY_BAN));
					break;
				}

				auth_nodes[csd->account_id].sex = csd->sex;
				auth_nodes[csd->account_id].char_id = char_id;
				auth_nodes[csd->account_id].gmlevel = csd->gmlevel;
				auth_nodes[csd->account_id].login_id1 = csd->login_id1;
				auth_nodes[csd->account_id].login_id2 = csd->login_id2;
				auth_nodes[csd->account_id].expiration_time = csd->expiration_time;

				WFIFOPACKET(cl, spacket, HC_NOTIFY_ZONESVR);
				spacket->char_id = char_id;
				maps.copy_map_name_ext((char*)spacket->map_name, cd.last_point.map);
				spacket->addr.ip = htonl(servers[server].addr.to_ulong());
				spacket->addr.port = servers[server].port;
				cl->send_buffer(sizeof(struct PACKET_HC_NOTIFY_ZONESVR));
			}
			break;

		case HEADER_CH_REQUEST_DEL_TIMER:
			FIFOSD_CHECK(6);
			delete2_req(cl, csd);
			cl->skip(6);
			break;

		case HEADER_CH_ACCEPT_DEL_REQ:
			FIFOSD_CHECK(12);
			delete2_accept(cl, csd);
			cl->skip(6);
			break;

		case HEADER_CH_CANCEL_DEL_REQ:
			FIFOSD_CHECK(6);
			delete2_cancel(cl, csd);
			cl->skip(6);
			break;

		case HEADER_CH_DELETE_CHAR:
		case HEADER_CH_DELETE_CHAR2:
			if (cmd == HEADER_CH_DELETE_CHAR) FIFOSD_CHECK(sizeof(struct PACKET_CH_DELETE_CHAR));
			if (cmd == HEADER_CH_DELETE_CHAR2) FIFOSD_CHECK(sizeof(struct PACKET_CH_DELETE_CHAR2));
			{
				int cid = RFIFOL(cl,2);
				char email[40];
				memcpy(email, RFIFOP(cl,6), 40);

				cl->skip((cmd == HEADER_CH_DELETE_CHAR) ? sizeof(struct PACKET_CH_DELETE_CHAR) : sizeof(struct PACKET_CH_DELETE_CHAR2));

				if (_strcmpi(email, csd->email) != 0 && (strcmp("a@a.com", csd->email) || (strcmp("a@a.com", email) && strcmp("", email))))
				{
					WFIFOPACKET(cl, spacket, HC_REFUSE_DELETECHAR); 
					spacket->error_code = 0;
					cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_DELETECHAR));
					break;
				}

				bool found = false;
				int i, ch;
				for (i = 0; i < MAX_CHARS; i++)
				{
					if (csd->found_char[i] == cid)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					WFIFOPACKET(cl, spacket, HC_REFUSE_DELETECHAR); 
					spacket->error_code = 0;
					cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_DELETECHAR));
					break;
				}
				else
				{
					for(ch = i; ch < MAX_CHARS - 1; ch++)
						csd->found_char[ch] = csd->found_char[ch+1];

					csd->found_char[MAX_CHARS - 1] = -1;

					if (!chars->delete_char(cid))
					{
						WFIFOPACKET(cl, spacket, HC_REFUSE_DELETECHAR); 
						spacket->error_code = 0;
						cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_DELETECHAR));
						break;
					}

					WFIFOPACKET(cl, spacket, HC_ACCEPT_DELETECHAR);
					cl->send_buffer(sizeof(struct PACKET_HC_ACCEPT_DELETECHAR));
				}
			}
			break;

		case HEADER_CH_MAKE_CHAR:
			FIFOSD_CHECK(sizeof(struct PACKET_CH_MAKE_CHAR));
			{
				TYPECAST_PACKET(RFIFOP(cl,0),rpacket,CH_MAKE_CHAR);

				// TODO: Check create char disabled
				int i = create_char(csd, (char*)rpacket->name,rpacket->str,rpacket->agi,rpacket->vit,rpacket->int_,rpacket->dex,rpacket->luk,rpacket->char_slot,rpacket->head_color,rpacket->head_style);

				//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
				if (i < 0)
				{
					WFIFOPACKET(cl, spacket, HC_REFUSE_MAKECHAR);

					switch (i) {
					case -1: spacket->error_code = 0x00; break;
					case -2: spacket->error_code = 0xFF; break;
					case -3: spacket->error_code = 0x01; break;
					}

					cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_MAKECHAR));
				}
				else
				{
					// retrieve data
					CharData char_dat;
					memset(&char_dat, 0, sizeof(CharData));
					chars->load_char(i, char_dat, false); //Only the short data is needed.

					// send to player
					WFIFOPACKET(cl, spacket, HC_ACCEPT_MAKECHAR);

					char_to_buf(&spacket->charinfo, &char_dat);

					cl->send_buffer(sizeof(struct PACKET_HC_ACCEPT_MAKECHAR));

					// add new entry to the chars list
					for (int n = 0; n < MAX_CHARS; n++)
					{
						if(csd->found_char[n] == -1)
							csd->found_char[n] = i; // the char_id of the new char
					}
				}
				cl->skip(sizeof(struct PACKET_CH_MAKE_CHAR));
			}
			break;

		case HEADER_CH_ENTER_CHECKBOT:
			FIFOSD_CHECK(sizeof(struct PACKET_CH_ENTER_CHECKBOT));
			{
				WFIFOPACKET(cl, spacket, HC_CHECKBOT_RESULT);
				spacket->packet_len = sizeof(struct PACKET_HC_CHECKBOT_RESULT);
				spacket->result = 1;
				cl->send_buffer(spacket->packet_len);
				cl->skip(TYPECAST_PACKET_ONCE(RFIFOP(cl,0), CH_ENTER_CHECKBOT)->packet_len);
			}
			break;

		case HEADER_CH_CHECKBOT:
			FIFOSD_CHECK(sizeof(struct PACKET_CH_CHECKBOT));
			{
				WFIFOPACKET(cl, spacket, HC_CHECKBOT_RESULT);
				spacket->packet_len = sizeof(struct PACKET_HC_CHECKBOT_RESULT);
				spacket->result = 1;
				cl->send_buffer(spacket->packet_len);
				cl->skip(TYPECAST_PACKET_ONCE(RFIFOP(cl,0), CH_CHECKBOT)->packet_len);
			}
			break;

		case HEADER_CH_ENTER:
			if(RFIFOREST(cl) < sizeof(struct PACKET_CH_ENTER))
				return 0;
			{
				int account_id = RFIFOL(cl,2);
				unsigned int login_id1 = RFIFOL(cl,6);
				unsigned int login_id2 = RFIFOL(cl,10);
				char sex = RFIFOB(cl,16);
				cl->skip(sizeof(struct PACKET_CH_ENTER));

				if (csd)
				{
					break;
				}

				csd = new CharSessionData();
				csd->account_id = account_id;
				csd->login_id1 = login_id1;
				csd->login_id2 = login_id2;
				csd->sex = sex;
				csd->auth = false;
				csd->cl = cl;
				cl->set_data((char*)csd);

				WFIFOHEAD(cl, 4);
				WFIFOL(cl,0) = account_id;
				cl->send_buffer(4);

				if (auth_nodes.count(account_id) && 
					auth_nodes[account_id].login_id1  == login_id1 &&
					auth_nodes[account_id].login_id2  == login_id2)
				{
					auth_nodes.erase(account_id);
					auth_ok(cl, csd);
				}
				else
				{
					if (auth_conn_ok)
					{
						WFIFOHEAD(auth_conn,19);
						WFIFOW(auth_conn,0) = INTER_CA_AUTH;
						WFIFOL(auth_conn,2) = csd->account_id;
						WFIFOL(auth_conn,6) = csd->login_id1;
						WFIFOL(auth_conn,10) = csd->login_id2;
						WFIFOB(auth_conn,14) = csd->sex;
						WFIFOL(auth_conn,15) = cl->tag();
						auth_conn->send_buffer(19);
					}
					else
					{
						WFIFOPACKET(cl, spacket, HC_REFUSE_ENTER);
						spacket->error_code = 0;
						cl->send_buffer(sizeof(struct PACKET_HC_REFUSE_ENTER));
					}
				}
			}
			break;

		case HEADER_PING:
			if (RFIFOREST(cl) < sizeof(PACKET_PING))
				return 0;
			cl->skip(sizeof(PACKET_PING));
			break;

		case INTER_ZC_LOGIN:
			if (RFIFOREST(cl) < 60)
				return 0;
			{
				char *user = (char*)RFIFOP(cl, 2);
				char *pass = (char*)RFIFOP(cl, 26);

				if (strcmp(user, config.inter_login_user.c_str()) || strcmp(pass, config.inter_login_pass.c_str()))
				{
					WFIFOHEAD(cl, 3);
					WFIFOW(cl, 0) = INTER_CZ_LOGIN_REPLY;
					WFIFOB(cl, 2) = 1;
					cl->send_buffer(3);
				}
				else
				{
					int id = cl->tag();
					servers[id].cl = cl;
					servers[id].addr = address_v4(ntohl(RFIFOL(cl, 54)));
					servers[id].port = ntohs(RFIFOW(cl, 58));
					servers[id].users = 0;
					
					cl->set_parser(&CharServer::parse_from_zone);
					cl->flags.server = 1;
					cl->realloc_fifo(FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

					WFIFOHEAD(cl, 3);
					WFIFOW(cl, 0) = INTER_CZ_LOGIN_REPLY;
					WFIFOB(cl, 2) = 0;
					cl->send_buffer(3);
				}

				cl->skip(60);
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


/*! 
 *  \brief     Connection success into char server
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
void CharServer::auth_ok(tcp_connection::pointer cl, CharSessionData *csd)
{
	if (online_chars.count(csd->account_id))
	{
		if (online_chars[csd->account_id].server > -1)
		{
			// TODO: Kick form ZoneServer

			if (online_chars[csd->account_id].disconnect_timer)
				TimerManager::FreeTimer(online_chars[csd->account_id].disconnect_timer);

			set_char_offline(csd->account_id, -1);

			WFIFOPACKET(cl, packet, SC_NOTIFY_BAN);
			packet->error_code = 8;
			cl->send_buffer(sizeof(struct PACKET_SC_NOTIFY_BAN));

			return;
		}

		if (online_chars[csd->account_id].cl->tag() != cl->tag())
		{
			WFIFOPACKET(cl, packet, SC_NOTIFY_BAN);
			packet->error_code = 8;
			cl->send_buffer(sizeof(struct PACKET_SC_NOTIFY_BAN));

			return;
		}

		online_chars[csd->account_id].cl = cl;
	}

	if (auth_conn_ok)
	{
		WFIFOHEAD(auth_conn,10);
		WFIFOW(auth_conn,0) = INTER_CA_REQ_ACC_DATA;
		WFIFOL(auth_conn,2) = csd->account_id;
		WFIFOL(auth_conn,6) = cl->tag();
		auth_conn->send_buffer(10);
	}

	set_charsel(csd->account_id, cl);
}

/*! 
 *  \brief     Character Selection
 *
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
void CharServer::set_charsel(int account_id, tcp_connection::pointer cl)
{
	// TODO: Decrement ZoneServer user on

	online_chars[account_id].char_id = -1;
	online_chars[account_id].server = -1;
	online_chars[account_id].cl = cl;

	if (online_chars[account_id].disconnect_timer) 
		TimerManager::FreeTimer(online_chars[account_id].disconnect_timer);
}

/*! 
 *  \brief     Send Characters
 *  \details   Send character informations to the client
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
void CharServer::send_chars(tcp_connection::pointer cl, CharSessionData *csd)
{
	int j = 0;
	WFIFOPACKET2(cl, packet, HC_ACCEPT_ENTER, MAX_CHARS * sizeof(CHARACTER_INFO));

#if PACKETVER >= 20100413
	packet->total_slots = MAX_CHARS_SLOTS;
	packet->premium_slots_start = MAX_CHARS;
	packet->premium_slots_end = MAX_CHARS;
#endif
	memset(packet->unknown, 0, sizeof(packet->unknown));
	packet->packet_len = chars->load_chars_to_buf(csd->account_id, packet->charinfo, csd) * sizeof(CHARACTER_INFO) + sizeof(PACKET_HC_ACCEPT_ENTER);
	
	cl->send_buffer(packet->packet_len);

	return;
}

/*! 
 *  \brief     Char to Buffer
 *  \details   Makes the character cache
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
void CharServer::char_to_buf(struct CHARACTER_INFO *charinfo, CharData *p)
{
	if( charinfo == NULL || p == NULL )
		return;

	charinfo->char_id = p->char_id;
	charinfo->base_exp = min<unsigned int>(p->base_exp, INT32_MAX);
	charinfo->zeny = p->zeny;
	charinfo->job_exp = min<unsigned int>(p->job_exp, INT32_MAX);
	charinfo->job_level = p->job_level;
	charinfo->bodystate = 0;
	charinfo->healthstate = 0;
	charinfo->effectstate = p->option;
	charinfo->virtue = p->karma;
	charinfo->honor = p->manner;
	charinfo->status_points = min<unsigned short>(p->status_point, INT16_MAX);
#if PACKETVER > 20081217
	charinfo->hp = p->hp;
	charinfo->max_hp = p->max_hp;
#else
	charinfo->hp = min<unsigned short>(p->hp, INT16_MAX);
	charinfo->max_hp = min<unsigned short>(p->max_hp, INT16_MAX);
#endif
	charinfo->sp = min<unsigned short>(p->sp, INT16_MAX);
	charinfo->max_sp = min<unsigned short>(p->max_sp, INT16_MAX);
	charinfo->speed = DEFAULT_WALK_SPEED; // p->speed;
	charinfo->class_ = p->class_;
	charinfo->head_style = p->hair;
	charinfo->weapon = p->option&0x7E80020 ? 0 : p->weapon; //When the weapon is sent and your option is riding, the client crashes on login!?
	charinfo->base_level = p->base_level;
	charinfo->skill_points = min<unsigned short>(p->skill_point, INT16_MAX);
	charinfo->head_bottom = p->head_bottom;
	charinfo->shield = p->shield;
	charinfo->head_top = p->head_top;
	charinfo->head_mid = p->head_mid;
	charinfo->head_color = p->hair_color;
	charinfo->body_color = p->clothes_color;
	memcpy(charinfo->name, p->name, NAME_LENGTH);
	charinfo->str = (unsigned char)min<unsigned short>(p->str, UINT8_MAX);
	charinfo->agi = (unsigned char)min<unsigned short>(p->agi, UINT8_MAX);
	charinfo->vit = (unsigned char)min<unsigned short>(p->vit, UINT8_MAX);
	charinfo->int_ = (unsigned char)min<unsigned short>(p->int_, UINT8_MAX);
	charinfo->dex = (unsigned char)min<unsigned short>(p->dex, UINT8_MAX);
	charinfo->luk = (unsigned char)min<unsigned short>(p->luk, UINT8_MAX);
	charinfo->char_slot = p->slot;
	charinfo->hair_color = 0; //Should we send it? It doesn't seem to affect anything.
#if PACKETVER >= 20061023
	charinfo->can_rename = ( p->rename > 0 ) ? 0 : 1;
#endif
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
	maps.copy_map_name_ext(charinfo->map_name, p->last_point.map);
#endif
#if PACKETVER >= 20100803
	charinfo->delete_date = TOL(p->delete_date);
#endif
#if PACKETVER >= 20110111
	charinfo->robe = p->robe;
#endif
#if PACKETVER >= 20110928
	charinfo->can_changeslot = 0;  // change slot feature (0 = disabled, otherwise enabled)
#endif
	return;
}

/*! 
 *  \brief     Create Character
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
int CharServer::create_char(CharSessionData *csd, char* name, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style)
{
	int char_id, flag;

	flag = check_char_name(name);
	if( flag < 0 )
		return flag;

	if((slot >= MAX_CHARS) // slots
		|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
		|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
		|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
		return -2; // invalid input

	// TODO: Check max chars per account

	{
		statement s = (database->prepare << "SELECT 1 FROM `char` WHERE `account_id`=:acc AND `char_num`=:slot", use(csd->account_id), use(slot));

		s.execute(false);

		if (s.get_affected_rows())
			return -2;
	}

	CharData c;
	memset(&c, 0, sizeof(CharData));
	c.account_id = csd->account_id;
	c.str = str;
	c.agi = agi;
	c.vit = vit;
	c.int_ = int_;
	c.dex = dex;
	c.luk = luk;
	strncpy(c.name, name, NAME_LENGTH);
	c.slot = slot;
	c.hair_color = hair_color;
	c.hair = hair_style;
	char_id = chars->save(c, true);

	// TODO: Add initial items

	return char_id;
}

/*! 
 *  \brief     Check Character Name
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
int CharServer::check_char_name(char *name)
{
	//int i;

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name

	// TODO: Check for reserved names/characters

	// check name (already in use?)
	{
		statement s = (database->prepare << "SELECT `char_id` FROM `char` WHERE `name`=:n", use(string(name)));

		s.execute(false);

		if (s.get_affected_rows())
			return -1;
	}

	return 0;
}

/*! 
 *  \brief     Check Email
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
bool CharServer::check_email(char *email)
{
	char ch;
	char* last_arobas;
	size_t len = strlen(email);

	// Server Limits
	if (len < 3 || len > 39)
		return 0;

	// Part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == NULL || email[len-1] == '@')
		return 0;

	if (email[len-1] == '.')
		return 0;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != NULL || strstr(last_arobas, "..") != NULL)
		return 0;

	for(ch = 1; ch < 32; ch++)
		if (strchr(last_arobas, ch) != NULL)
			return 0;

	if (strchr(last_arobas, ' ') != NULL || strchr(last_arobas, ';') != NULL)
		return 0;

	// All Correct
	return 1;
}

/*! 
 *  \brief     Delete Character Requisition
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      10/12/11
 *
 **/
void CharServer::delete2_req(tcp_connection::pointer cl, CharSessionData *csd)
{
	int char_id, i;
	time_t delete_date;

	char_id = RFIFOL(cl, 2);

	bool found = false;
	//int ch;
	for (i = 0; i < MAX_CHARS; i++)
	{
		if (csd->found_char[i] == char_id)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		delete2_ack(cl, char_id, 3, 0);
		return;
	}
	else
	{
		// TODO: Implement char deletion delay
		delete_date = time(NULL) + 60;

		*database << "UPDATE `char` SET `delete_date`=:t WHERE `char_id`=:c", use(delete_date), use(char_id);

		delete2_ack(cl, char_id, 1, delete_date);
	}
}

/*! 
 *  \brief     Delete character OK
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      10/12/11
 *
 **/
void CharServer::delete2_accept( tcp_connection::pointer cl, CharSessionData * csd )
{
	char birthdate[8+1];
	int char_id, i, k;
	//unsigned int base_level;
	//char* data;
	time_t delete_date;

	char_id = RFIFOL(cl,2);

	ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", csd->account_id, char_id);

	// construct "YY-MM-DD"
	birthdate[0] = RFIFOB(cl,6);
	birthdate[1] = RFIFOB(cl,7);
	birthdate[2] = '-';
	birthdate[3] = RFIFOB(cl,8);
	birthdate[4] = RFIFOB(cl,9);
	birthdate[5] = '-';
	birthdate[6] = RFIFOB(cl,10);
	birthdate[7] = RFIFOB(cl,11);
	birthdate[8] = 0;

	bool found = false;
	for (i = 0; i < MAX_CHARS; i++)
	{
		if (csd->found_char[i] == char_id)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		delete2_accept_ack(cl, char_id, 3);
		return;
	}

	*database << "SELECT `delete_date` FROM `char` WHERE `char_id` = :c LIMIT 1", use(char_id), into(delete_date);

	if( !delete_date || delete_date>time(NULL) )
	{// not queued or delay not yet passed
		delete2_accept_ack(cl, char_id, 4);
		return;
	}

	if(strcmp(csd->birthdate + 2, birthdate))  // +2 to cut off the century
	{
		delete2_accept_ack(cl, char_id, 5);
		return;
	}

	if(!chars->delete_char(char_id))
	{
		delete2_accept_ack(cl, char_id, 3);
		return;
	}

	for(k = i; k < MAX_CHARS - 1; k++)
		csd->found_char[k] = csd->found_char[k+1];

	csd->found_char[MAX_CHARS - 1] = -1;

	delete2_accept_ack(cl, char_id, 1);
}

/*! 
 *  \brief     Cancel Character Delection
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      10/12/11
 *
 **/
void CharServer::delete2_cancel( tcp_connection::pointer cl, CharSessionData * csd )
{
	int char_id, i;

	char_id = RFIFOL(cl,2);

	bool found = false;
	for (i = 0; i < MAX_CHARS; i++)
	{
		if (csd->found_char[i] == char_id)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		delete2_accept_ack(cl, char_id, 3);
		return;
	}

	// there is no need to check, whether or not the character was
	// queued for deletion, as the client prints an error message by
	// itself, if it was not the case (@see char_delete2_cancel_ack)
	*database << "UPDATE `char` SET `delete_date` = 0 WHERE `char_id`=:c", use(char_id);

	delete2_cancel_ack(cl, char_id, 1);
}

void CharServer::delete2_ack( tcp_connection::pointer cl, int char_id, int result, time_t deltime )
{
	WFIFOHEAD(cl,14);
    WFIFOW(cl,0) = HEADER_HC_DEL_REQUEST_ACK;
	WFIFOL(cl,2) = char_id;
	WFIFOL(cl,6) = result;
	WFIFOL(cl,10) = TOL(deltime);
	cl->send_buffer(14);
}

void CharServer::delete2_accept_ack( tcp_connection::pointer cl, int char_id, int result )
{
	WFIFOHEAD(cl,10);
	WFIFOW(cl,0) = HEADER_HC_DEL_ACCEPT_ACK;
	WFIFOL(cl,2) = char_id;
	WFIFOL(cl,6) = result;
	cl->send_buffer(10);
}

void CharServer::delete2_cancel_ack( tcp_connection::pointer cl, int char_id, int result )
{
	WFIFOHEAD(cl,10);
	WFIFOW(cl,0) = HEADER_HC_DEL_CANCEL_ACK;
	WFIFOL(cl,2) = char_id;
	WFIFOL(cl,6) = result;
	cl->send_buffer(10);
}
