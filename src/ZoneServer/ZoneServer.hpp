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
#include "BlockManager.hpp"
#include "PlayerModules.hpp"

using namespace std;

#define MAX_PACKET_DB 0x900
#define MAX_PACKET_POS 20

enum SessionState 
{ 
	ST_LOGIN, 
	ST_LOGOUT, 
	ST_MAPCHANGE 
};

/*! 
 *  \brief     Zone Server Authorization node
 *
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      ??/12/11
 *
 **/
struct AuthNode
{
	AuthNode()
	{
		sd = 0;
		char_dat = 0;
	}

	int account_id, char_id;
	int login_id1, login_id2, sex;
	time_t expiration_time;
	struct ZoneSessionData *sd;
	struct CharData *char_dat;
	unsigned int node_created;
	enum SessionState auth_state;

	tcp_connection::pointer cl;
};

/*! 
 *  \brief     Online Char Informations
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      ??/12/11
 *
 **/
struct OnlineChar 
{
	int account_id;
	int char_id;

	tcp_connection::pointer cl;
	int disconnect_timer;
};

typedef boost::function<void (tcp_connection::pointer, ZoneSessionData *)> PacketCallback;
struct PacketData
{
	short len;
	PacketCallback callback;
	unsigned short pos[MAX_PACKET_POS];
};

#define addpacket(id, size, func, ...) ZoneServer::client_add_packet(id, size, func, __VA_ARGS__, 0xFFFF)

/*! 
 *  \brief     Zone Server Main Class
 *  \details   Zone Server common modules
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      ??/12/11
 *
 **/
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
	
	// Char InterConn
	static void connect_to_char();
	static void send_maps();
	static void inter_confirm_auth(ZoneSessionData *sd);
	static void create_auth_entry(ZoneSessionData *sd, enum SessionState state);

	// Client
	static PacketData *client_packets[MAX_PACKET_DB];
	static void set_char_offline(int account_id, char char_id);
	static void disconnect_timeout(int timer, int accid);

	static void init_packets();
	static void client_add_packet(unsigned short id, short size, PacketCallback func, ...);

	static void auth_fail(tcp_connection::pointer cl, int err);
	static void auth_ok( ZoneSessionData * sd );

	static void request_registry( ZoneSessionData * sd, int flag );
	static void clif_spawn( struct BlockList* bl );
	static void addblock( struct BlockList* bl );

	// Char InterConn
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
};
