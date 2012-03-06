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
*              Character Server Structures and classes          	*
* ==================================================================*/

#pragma once

#include  "../Common/config_file.hpp"
#include  "../Common/tcp_server.h"
#include  "../Common/ragnarok.hpp"
#include  "../Common/map_index.hpp"


#include <string>
#include <soci/soci.h>
#include <vector>

using namespace std;

#define MAX_CHAR_BUF 140

/*! 
 *  \brief     Character Session Data
 *  \details   Char Server Main informations
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
struct CharSessionData
{
	bool auth;
	int account_id, login_id1, login_id2, sex;

	char email[40];
	time_t expiration_time;
	int gmlevel;

	int found_char[MAX_CHARS];

	char birthdate[11];

	tcp_connection::pointer cl;
};

///! \brief Authorization node
struct AuthNode
{
	int char_id;

	int login_id1;
	int login_id2;
	int gmlevel;
	time_t expiration_time;

	char sex;
};

///! Online Character informations
struct OnlineChar 
{
	int account_id;
	int char_id;

	tcp_connection::pointer cl;
	int disconnect_timer;
	int server;
};

///! Zone Server Inter Connection Informations
struct ZoneServerConnection
{
	boost::asio::ip::address_v4 addr;
	unsigned short port;

	tcp_connection::pointer cl;

	unsigned int users;
};

/*! 
 *  \brief     Char Server Main Class
 *  \details   Char Server Modules
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
class CharDB;
class CharServer
{
public:
	typedef std::map<int, struct AuthNode> auth_node_db;
	typedef std::map<int, struct OnlineChar> online_char_db;
	typedef std::map<int, struct ZoneServerConnection> zone_server_db;

	struct login_config
	{
		// CharServer
		string			server_name;

		// Network
		string			network_bindip;
		unsigned short	network_bindport;

		// IP to sent to AuthServer
		string			network_charip;

		// Interconnection
		string			inter_login_ip;
		unsigned short	inter_login_port;
		string			inter_login_user;
		string			inter_login_pass;
	}; 

	static void run();
	static int parse_from_client(tcp_connection::pointer cl);
	static int parse_from_zone(tcp_connection::pointer cl);
	static int parse_from_login(tcp_connection::pointer cl);
	
	// Auth InterConn
	static void connect_to_auth();
	static void auth_ok(tcp_connection::pointer cl, CharSessionData *csd);
	static void set_charsel(int account_id, tcp_connection::pointer cl);

	// Client
	static void set_char_offline(int account_id, char char_id);
	static void set_char_online(int server, int char_id, int account_id);
	static void disconnect_timeout(int timer, int accid);

	static void send_chars(tcp_connection::pointer cl, CharSessionData *csd);
	static void char_to_buf(struct CHARACTER_INFO *charinfo, CharData *p);
	static int create_char(CharSessionData *csd, char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style);
	static int check_char_name(char *name);
	static bool check_email(char *email);

	static void delete2_req( tcp_connection::pointer cl, CharSessionData *csd );
	static void delete2_accept( tcp_connection::pointer cl, CharSessionData * csd );
	static void delete2_cancel( tcp_connection::pointer cl, CharSessionData * csd );
	static void delete2_ack( tcp_connection::pointer cl, int char_id, int result, time_t deltime );
	static void delete2_accept_ack( tcp_connection::pointer cl, int char_id, int param3 );
	static void delete2_cancel_ack( tcp_connection::pointer cl, int char_id, int result );
	static void inter_reply_regs( tcp_connection::pointer cl, int account_id, int char_id, int type );
	static void request_accreg2( int account_id, int char_id );

	static bool auth_conn_ok;
	static tcp_connection::pointer auth_conn;

	// Config
	static config_file *char_config;

	static struct login_config config;

	// Network
	static boost::asio::io_service *io_service;
	static tcp_server *server;

	// Database
	static soci::session *database;
	static CharDB *chars;

	// Auth
	static auth_node_db auth_nodes;
	static online_char_db online_chars;

	// Maps
	static map_index maps;
	static map<int, int> map_to_zone;

	// Zone
	static zone_server_db servers;
};

#include "CharDB.hpp"
