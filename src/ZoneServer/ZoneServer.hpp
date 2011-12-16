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

#include <config_file.hpp>
#include <tcp_server.hpp>
#include <string>
#include <soci/soci.h>
#include <ragnarok.hpp>
#include <map_index.hpp>

using namespace std;

#define MAX_CHAR_BUF 140

struct ZoneSessionData
{
	bool auth;

	tcp_connection::pointer cl;
};

struct AuthNode
{
	int login_id1;
	int login_id2;

	char sex;
};

struct OnlineChar 
{
	int account_id;
	int char_id;

	tcp_connection::pointer cl;
	int disconnect_timer;
};

class ZoneServer
{
public:
	typedef std::map<int, struct AuthNode> auth_node_db;
	typedef std::map<int, struct OnlineChar> online_account_db;

	struct login_config
	{
		// Network
		string			network_bindip;
		unsigned short	network_bindport;

		// IP to sent to CharServer
		string			network_zoneip;

		// Interconnection
		string			inter_char_ip;
		unsigned short	inter_char_port;
		string			inter_char_user;
		string			inter_char_pass;
	}; 

	static void run();
	static int parse_from_client(tcp_connection::pointer cl);
	static int parse_from_char(tcp_connection::pointer cl);
	
	// Auth InterConn
	static void connect_to_char();

	// Client
	static void set_char_offline(int account_id, char char_id);
	static void disconnect_timeout(int timer, int accid);

	static bool char_conn_ok;
	static tcp_connection::pointer char_conn;

	// Config
	static config_file *char_config;

	static struct login_config config;

	// Network
	static boost::asio::io_service *io_service;
	static tcp_server *server;

	// Database
	static soci::session *database;

	// Auth
	static auth_node_db auth_nodes;
	static online_account_db online_chars;

	// Maps
	static map_index maps;
};
