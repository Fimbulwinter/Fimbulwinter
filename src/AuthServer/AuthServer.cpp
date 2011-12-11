/*============ Cronus++ developer team presents: ==========*
*	______ _           _           _           _		   *
*	|  ___(_)         | |         | |         | |		   *
*	| |_   _ _ __ ___ | |__  _   _| |_   _____| |_ _ __    *
*	|  _| | | '_ ` _ \| '_ \| | | | \ \ / / _ \ __| '__|   *
*	| |   | | | | | | | |_) | |_| | |\ V /  __/ |_| |      *
*	\_|   |_|_| |_| |_|_.__/ \__,_|_| \_/ \___|\__|_|      *
* -------------------------------------------------------- *
*               An Ragnarok Online Emulator                *
* -------------------------------------------------------- *
*                Licenced under GNU GPL v3                 *
* -------------------------------------------------------- *
*                  Authentication Server				   *
* ======================================================== */

#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <packets.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>

#include <md5.hpp>
#include <strfuncs.hpp>
#include <boost/foreach.hpp>


// Config
config_file *AuthServer::auth_config;
config_file *AuthServer::database_config;

struct AuthServer::login_config AuthServer::config;

// Network
tcp_server *AuthServer::server;

// Database
soci::session *AuthServer::database;
AccountDB *AuthServer::accounts;

// Interconnection
AuthServer::char_server_db AuthServer::servers;

/*==============================================================*
* Function:	Auth::Run											*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Start Auth-Server and load configurations        *
**==============================================================*/
void AuthServer::run()
{
	boost::asio::io_service io_service;

	// Read Config Files
	try
	{
		auth_config = new config_file("./Config/authserver.conf");
		{
			config.network_bindip = auth_config->read<string>("network.bindip", "0.0.0.0");
			config.network_bindport = auth_config->read<unsigned short>("network.bindport", 6900);
			config.auth_use_md5 = auth_config->read<bool>("auth.usemd5", false);
		}
		ShowStatus("Finished reading authserver.conf.\n");

		database_config = new config_file("./Config/database.conf");
		{
			// Validate something?
		}
		ShowStatus("Finished reading database.conf.\n");
	}
	catch (config_file::file_not_found *fnf)
	{
		ShowFatalError("Config file not found: %s.\n", fnf->filename);
		return;
	}

	TimerManager::Initialize(&io_service);

	// Initialize Database System
	{
		ShowInfo("Opening connection to database...\n");

		try
		{
			database = database_helper::get_session(database_config);
		}
		catch (soci::soci_error err)
		{
			ShowFatalError("Error opening database connection: %s\n", err.what());
			return;
		}

		accounts = new AccountDB(database);

		ShowSQL("Successfully opened database connection.\n");
	}

	// Initialize Network System
	{
		boost::system::error_code err;
		address_v4 bindip = address_v4::from_string(config.network_bindip, err);

		if (err)
		{
			ShowFatalError("%s\n", err.message().c_str());
			return;
		}

		server = new tcp_server(io_service, (address)bindip, config.network_bindport);
		server->set_default_parser(AuthServer::parse_from_client);

		ShowStatus("AuthServer is ready and listening on %s:%d.\n", config.network_bindip.c_str(), config.network_bindport);
	}

	// Run IO service service and start pooling events
	io_service.run();
}

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	core_display_title();

	AuthServer::run();

	getchar();
	return 0;
}
