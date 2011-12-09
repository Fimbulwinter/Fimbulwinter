#pragma once

#include <config_file.hpp>
#include <tcp_server.hpp>
#include <string>

#include <soci/soci.h>

#include <ragnarok.hpp>
#include "AccountDB.h"
#include <map>

using namespace std;

enum auth_type
{
	auth_raw,
	auth_md5,
	auth_token,
};

struct AuthSessionData
{
	int account_id;
	long login_id1;
	long login_id2;
	char sex;// 'F','M','S'

	char username[NAME_LENGTH];
	char password[32+1];
	enum auth_type type;

	char md5key[20];
	unsigned short md5keylen;

	char lastlogin[24];
	unsigned char level;
	unsigned char clienttype;
	unsigned int version;

	bool gameguardChallenged;

	tcp_connection::pointer cl;
};

struct CharServerConnection
{
	int account_id;

	boost::asio::ip::address_v4 addr;
	unsigned short port;

	char name[20];
	tcp_connection::pointer cl;
};

class AuthServer
{
public:
	struct login_config
	{
		// Network
		string			network_bindip;
		unsigned short	network_bindport;

		// Password Types
		static int useMD5; 
		//int OTP; [TODO]
	};

	static void run();
	static int parse_from_client(tcp_connection::pointer cl);
	static int parse_from_char(tcp_connection::pointer cl);

	// Auth
	static int authenticate(AuthSessionData *asd);
	static bool check_auth(const char *md5key, enum auth_type type, const char *passwd, const char *refpass);
	static void send_auth_err(AuthSessionData *asd, int result);
	static void send_auth_ok(AuthSessionData *asd);

	// Config
	static config_file *auth_config;
	static config_file *database_config;

	static struct login_config config;

	// Network
	static tcp_server *server;

	// Database
	static soci::session *database;
	static AccountDB *accounts;

	// Interconnection
	static std::map<int, struct CharServerConnection> servers;
private:
};
