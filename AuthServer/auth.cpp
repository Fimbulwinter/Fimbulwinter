#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <timers.hpp>

#include <iostream>

const char *timestamp2string(char *str, size_t size, time_t timestamp, const char *format)
{
	size_t len = strftime(str, size, format, localtime(&timestamp));
	memset(str + len, '\0', size - len);
	return str;
}

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

void AuthServer::send_auth_err(AuthSessionData *asd, int result)
{
	tcp_connection::pointer cl = asd->cl;
	string ip = asd->cl->socket().remote_endpoint().address().to_string();

	WFIFOHEAD(cl,23);
	WFIFOW(cl,0) = 0x6a;
	WFIFOB(cl,2) = (unsigned char)result;
	if( result != 6 )
	{
		memset(WFIFOP(cl,3), '\0', 20);
	}
	else
	{
		Account acc;
		time_t unban_time = ( accounts->load_account(asd->username, acc) ) ? acc.unban_time : 0;
		
		timestamp2string((char*)WFIFOP(cl,3), 20, unban_time, "%Y-%m-%d %H:%M:%S");
	}

	cl->send_buffer(23);
}

void AuthServer::send_auth_ok(AuthSessionData *asd)
{
	WFIFOHEAD(asd->cl,3);
	WFIFOW(asd->cl,0) = 0x81;
	WFIFOB(asd->cl,2) = 1; // 01 = Server closed
	asd->cl->send_buffer(3);
}

bool AuthServer::check_auth(const char *md5key, enum auth_type type, const char *passwd, const char *refpass)
{
	if (type == auth_raw)
	{
		return strcmp(passwd, refpass) == 0;
	}
	else if (type == auth_md5)
	{
		// TODO: Add support to MD5 hashed passwords
	}
	else if (type == auth_token)
	{
		// TODO: Add support to token-based authentication
	}

	return false;
}
