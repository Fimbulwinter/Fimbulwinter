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
*                      Authentication Systems              	        *
* ==================================================================*/

#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <packets.hpp>
#include <timers.hpp>
#include <md5.hpp>
#include <boost/foreach.hpp>
#include <iostream>

AuthServer::auth_node_db AuthServer::auth_nodes;
AuthServer::online_account_db AuthServer::online_accounts;

/*==============================================================*
* Function:	TimeStamp to String function						*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Save a timestamp in string format                *
**==============================================================*/
const char *timestamp2string(char *str, size_t size, time_t timestamp, const char *format)
{
	size_t len = strftime(str, size, format, localtime(&timestamp));
	memset(str + len, '\0', size - len);
	return str;
}

/*==============================================================*
* Function:	AuthServer Autentication							*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Authenticate a user on the AuthServer            *
**==============================================================*/
int AuthServer::authenticate(AuthSessionData *asd)
{
	Account acc;
	string ip = asd->cl->socket().remote_endpoint().address().to_string();

	if (!accounts->load_account(asd->username, acc))
	{
		ShowNotice("Unknown account (account: %s, received pass: %s, ip: %s)\n", asd->username, asd->password, ip.c_str());
		
		return 0;
	}

	if (!check_auth(asd->md5key, asd->type, asd->password, acc.user_pass.c_str()))
	{
		ShowNotice("Invalid password (account: '%s', pass: '%s', received pass: '%s', ip: %s)\n", asd->username, acc.user_pass.c_str(), asd->password, ip.c_str());
		
		return 1;
	}

	if (acc.expiration_time != 0 && acc.expiration_time < time(NULL))
	{
		ShowNotice("Connection refused (account: %s, pass: %s, expired ID, ip: %s)\n", asd->username, asd->password, ip.c_str());
		
		return 2;
	}

	if (acc.unban_time != 0 && acc.unban_time > time(NULL))
	{
		char tmpstr[24];
		timestamp2string(tmpstr, sizeof(tmpstr), acc.unban_time, "%Y-%m-%d %H:%M:%S");
		ShowNotice("Connection refused (account: %s, pass: %s, banned until %s, ip: %s)\n", asd->username, asd->password, tmpstr, ip.c_str());

		return 6;
	}

	if(acc.state != 0)
	{
		ShowNotice("Connection refused (account: %s, pass: %s, state: %d, ip: %s)\n", asd->username, asd->password, acc.state, ip.c_str());
		
		return acc.state - 1;
	}

	ShowNotice("Authentication accepted (account: %s, id: %d, ip: %s)\n", asd->username, acc.account_id, ip.c_str());

	asd->account_id = acc.account_id;
	asd->login_id1 = rand();
	asd->login_id2 = rand();

	strncpy(asd->lastlogin, acc.lastlogin.c_str(), sizeof(asd->lastlogin));

	asd->sex = acc.sex;
	asd->level = acc.level;

	timestamp2string((char *)acc.lastlogin.c_str(), sizeof(asd->lastlogin), time(NULL), "%Y-%m-%d %H:%M:%S");
	acc.last_ip = ip;
	acc.unban_time = 0;
	acc.logincount++;

	accounts->save_account(acc, false);

	return -1;
}

/*==============================================================*
* Function:	Authentication Error Function						*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Send to the client an anthentication 			*
* error message          										*
**==============================================================*/
void AuthServer::send_auth_err(AuthSessionData *asd, int result)
{
	tcp_connection::pointer cl = asd->cl;
	string ip = asd->cl->socket().remote_endpoint().address().to_string();

	WFIFOPACKET(cl, packet, AC_REFUSE_LOGIN);
	packet->error_code = result;
	if(result != 6)
	{
		memset(packet->block_date, '\0', 20);
	}
	else
	{
		Account acc;
		time_t unban_time = ( accounts->load_account(asd->username, acc) ) ? acc.unban_time : 0;
		
		timestamp2string((char*)packet->block_date, 20, unban_time, "%Y-%m-%d %H:%M:%S");
	}

	cl->send_buffer(sizeof(struct PACKET_AC_REFUSE_LOGIN));
}

/*==============================================================*
* Function:	AuthServer Autentication Success					*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Connect the user into the auth server            *
**==============================================================*/
void AuthServer::send_auth_ok(AuthSessionData *asd)
{
	tcp_connection::pointer cl = asd->cl;
	int server_num = servers.size();
	int n = 0;

	if (online_accounts.count(asd->account_id))
	{
		if (online_accounts[asd->account_id].char_server > -1)
		{
			unsigned char buf[6];

			WBUFW(buf,0) = INTER_AC_KICK;
			WBUFL(buf,2) = asd->account_id;
			char_sendallwos(-1, buf, 6);

			if (online_accounts[asd->account_id].enter_charserver_timeout)
				TimerManager::FreeTimer(online_accounts[asd->account_id].enter_charserver_timeout);

			shutdown_account(asd->account_id);

			WFIFOPACKET(asd->cl, packet, SC_NOTIFY_BAN);
			packet->error_code = 8;
			asd->cl->send_buffer(sizeof(struct PACKET_SC_NOTIFY_BAN));
		}
		else if (online_accounts[asd->account_id].char_server == -1)
		{
			shutdown_account(asd->account_id);
		}
	}

	if (server_num == 0)
	{
		WFIFOPACKET(asd->cl, packet, SC_NOTIFY_BAN);
		packet->error_code = 1;
		asd->cl->send_buffer(sizeof(struct PACKET_SC_NOTIFY_BAN));// 01 = Server closed
	}

	if(asd->level > 0)
		ShowStatus("Connection of the GM (level: %d) account '%s' accepted.\n", asd->level, asd->username);
	else
		ShowStatus("Connection of the account '%s' accepted.\n", asd->username);

	WFIFOPACKET2(asd->cl, packet, AC_ACCEPT_LOGIN, sizeof(struct PACKET_AC_ACCEPT_LOGIN::CHAR_SERVER_INFO) * server_num);
	packet->packet_len = sizeof(struct PACKET_AC_ACCEPT_LOGIN) + sizeof(struct PACKET_AC_ACCEPT_LOGIN::CHAR_SERVER_INFO) * server_num;
	packet->auth_code = asd->login_id1;
	packet->account_id = asd->account_id;
	packet->user_level = asd->login_id2;
	packet->lastlogin_ip = 0;
	memset(packet->lastlogin_time, 0, sizeof(packet->lastlogin_time));
	packet->sex = sex_str2num(asd->sex);

	map<int, CharServerConnection>::iterator it;
	for(it = servers.begin(); it != servers.end(); it++)
	{
		packet->server_info[n].ip_address = htonl(it->second.addr.to_ulong());
		packet->server_info[n].port = ntohs(htons(it->second.port));
		memcpy(packet->server_info[n].name, it->second.name, 20);
		packet->server_info[n].user_count = 0; // Users online
		packet->server_info[n].state = 0; // Server Type
		packet->server_info[n].property_ = 0; // Mark as new server?
		n++;
	}
	cl->send_buffer(packet->packet_len);

	// Creates an temporary authentication node
	auth_nodes[asd->account_id].login_id1 = asd->login_id1;
	auth_nodes[asd->account_id].login_id2 = asd->login_id2;
	auth_nodes[asd->account_id].sex = asd->sex;

	// None CharServer selected yet
	online_accounts[asd->account_id].char_server = -1;

	// Adds an timeout to user select an CharServer
	online_accounts[asd->account_id].enter_charserver_timeout = TimerManager::CreateStartTimer(AUTH_TIMEOUT, 
		false, boost::bind(&AuthServer::select_charserver_timeout, 
		_1, asd->account_id));
}

/*==============================================================*
* Function:	MD5 Password Check									*                                                     
* Author: Minos	                                                *
* Date: 09/12/11 												*
* Description: Check a md5 password					            *
**==============================================================*/
bool md5check(const char* str1, const char* str2, const char* passwd)
{
	char md5str[64+1];

	strcpy(md5str,str1);
	strcat(md5str,str2);
	if (!md5(md5str, md5str))
		return false;

	return !(strcmp(passwd,md5str));
}

/*==============================================================*
* Function:	Authentication Check								*                                                     
* Author: GreenBox/Minos                                        *
* Date: 09/12/11 												*
* Description: Check the auth type(PlainText/MD5/Token)			*
**==============================================================*/
bool AuthServer::check_auth(const char *md5key, enum auth_type type, const char *passwd, const char *refpass)
{
	if (type == auth_raw)
	{
		return strcmp(passwd, refpass) == 0;
	}
	else if (type == auth_md5)
	{
		return (/*(auth_md5&0x01) && */md5check(md5key, refpass, passwd)) ||
		       (/*(auth_md5&0x02) && */md5check(refpass, md5key, passwd));
	}
	else if (type == auth_token)
	{
		// TODO: Add support to token-based authentication
	}

	return false;
}

/*==============================================================*
* Function:	Select CharServer Timeout							*                                                     
* Author: GreenBox		                                        *
* Date: 10/12/11 												*
* Description: Occurs when the client don't select a CharServer *
* in given time.												*
**==============================================================*/
void AuthServer::select_charserver_timeout(int timer, int accid)
{
	shutdown_account(accid);
}

/*==============================================================*
* Function:	Shutdown Account									*                                                     
* Author: GreenBox		                                        *
* Date: 10/12/11 												*
* Description: Remove and account from online list and			*
* remove it from the authentication node						*
**==============================================================*/
void AuthServer::shutdown_account(int accid)
{
	if (online_accounts.count(accid))
		online_accounts.erase(accid);

	if (auth_nodes.count(accid))
		auth_nodes.erase(accid);
}
