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
		// Network
		string			network_bindip;
		unsigned short	network_bindport;
	}; 

	static void run();
	static int parse_from_client(tcp_connection::pointer cl);

	// Config
	static config_file *auth_config;
	static config_file *database_config;

	static struct login_config config;

	// Network
	static tcp_server *server;

	// Database
	static soci::session *database;
private:
};
