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
*             Authentication Server Structures and Classes 	        *
* ==================================================================*/

#pragma once

#include <config_file.hpp>
#include <tcp_server.hpp>
#include <string>

#include <soci/soci.h>

#include <ragnarok.hpp>
#include "AccountDB.h"
#include <map>

using namespace std;

#define AUTH_TIMEOUT 30000

enum auth_type
{
	auth_raw,
	auth_md5,
	auth_token,
};

/*==============================================================*
* Structure: Authorization Server Data							*
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Structure responsible for basic account          * 
* informations.                                                 *
**==============================================================*/
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


/*==============================================================*
* Structure: Character server connection data					*
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Structure responsible for char-server	        * 
* connection.                                                   *
**==============================================================*/
struct CharServerConnection
{
	int account_id;

	boost::asio::ip::address_v4 addr;
	unsigned short port;

	char name[20];
	tcp_connection::pointer cl;
};

/*==============================================================*
* Structure: Authorization Node									*
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description:											        * 
**==============================================================*/
struct AuthNode
{
	int login_id1;
	int login_id2;
	char sex;
};

/*==============================================================*
* Structure: Online Account Info								*
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description:											        * 
**==============================================================*/
struct OnlineAccount
{
	int char_server;
	int enter_charserver_timeout;
};

/*==============================================================*
* Class: Authorization Server Data								*
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Class responsible for general Auth-Server		* 
* informations.                                                 *
**==============================================================*/
class AuthServer
{
public:
	typedef std::map<int, struct OnlineAccount> online_account_db;
	typedef std::map<int, struct AuthNode> auth_node_db;
	typedef std::map<int, struct CharServerConnection> char_server_db;

	struct login_config
	{
		// Network
		string			network_bindip;
		unsigned short	network_bindport;

		// Authentication Types/Settings
		bool auth_database;
		bool auth_use_md5;
		//int OTP; [TODO]
	};

	static void run();
	static int parse_from_client(tcp_connection::pointer cl);
	static int parse_from_char(tcp_connection::pointer cl);

	// Auth
	static int authenticate(AuthSessionData *asd);
	static void send_auth_err(AuthSessionData *asd, int result);
	static void send_auth_ok(AuthSessionData *asd);

	static bool check_auth(const char *md5key, enum auth_type type, const char *passwd, const char *refpass);

	static void select_charserver_timeout(int timer, int accid);
	static void shutdown_account(int account_id);

	// Inter
	static void char_sendallwos(int cs, unsigned char *buf, size_t len);

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
	static char_server_db servers;
	static auth_node_db auth_nodes;
	static online_account_db online_accounts;
private:
};
