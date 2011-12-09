#pragma once

#include <config_file.hpp>
#include <tcp_server.hpp>
#include <string>

#include <soci/soci.h>

#include <ragnarok.hpp>

using namespace std;

enum auth_type
{
	auth_raw,
	auth_md5,
	auth_token,
};

class CharServer
{
public:
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
	
	// Login InterConn
	static void connect_to_auth();
	static tcp_connection::pointer auth_conn;

	// Config
	static config_file *auth_config;
	static config_file *database_config;

	static struct login_config config;

	// Network
	static boost::asio::io_service *io_service;
	static tcp_server *server;

	// Database
	static soci::session *database;
private:
};
