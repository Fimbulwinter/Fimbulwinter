/*=========================================================*
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
*					 Character Server					   *
* ======================================================== */

#include "CharServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <boost/thread.hpp>
#include <ragnarok.hpp>
#include <packets.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>
#include <boost/foreach.hpp>
#include <strfuncs.hpp>

// Login InterConn
tcp_connection::pointer CharServer::auth_conn;

// Config
config_file *CharServer::auth_config;
config_file *CharServer::database_config;

struct CharServer::login_config CharServer::config;

// Network
boost::asio::io_service *CharServer::io_service;
tcp_server *CharServer::server;

// Database
soci::session *CharServer::database;
CharDB *CharServer::chars;

// Auth
CharServer::auth_node_db CharServer::auth_nodes;
CharServer::online_account_db CharServer::online_chars;
bool CharServer::auth_conn_ok;

/*==============================================================*
* Function:	Start Char Server									*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Start the char-server and load the confs         *
**==============================================================*/
void CharServer::run()
{
	io_service = new boost::asio::io_service();

	// Read Config Files
	try
	{
		auth_config = new config_file("./Config/charserver.conf");
		{
			config.server_name = auth_config->read<string>("server.name", "Cronus++");

			config.network_bindip = auth_config->read<string>("network.bindip", "0.0.0.0");
			config.network_bindport = auth_config->read<unsigned short>("network.bindport", 6121);

			config.network_charip = auth_config->read<string>("network.charip", "");

			config.inter_login_ip = auth_config->read<string>("inter.login.ip", "127.0.0.1");
			config.inter_login_port = auth_config->read<unsigned short>("inter.login.port", 6900);
			config.inter_login_user = auth_config->read<string>("inter.login.user", "s1");
			config.inter_login_pass = auth_config->read<string>("inter.login.pass", "p1");

			if (config.network_charip == "")
			{
				ShowInfo("Auto-detecting my IP Address...\n");
				
				tcp::resolver resolver(*io_service);
				tcp::resolver::query query(boost::asio::ip::host_name(), "");
				tcp::resolver::iterator iter = resolver.resolve(query);
				tcp::resolver::iterator end; // End marker.
				while (iter != end)
				{
					tcp::endpoint ep = *iter++;

					if (!ep.address().is_v4())
						continue;
					
					config.network_charip = ep.address().to_string();

					break;
				}

				ShowStatus("Defaulting our IP Address to %s...\n", config.network_charip.c_str());
			}
		}
		ShowStatus("Finished reading charserver.conf.\n");

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

	TimerManager::Initialize(io_service);

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

		chars = new CharDB(database);

		ShowSQL("Successfully opened database connection.\n");
	}

	connect_to_auth();

	// Initialize Network System
	{
		boost::system::error_code err;
		address_v4 bindip = address_v4::from_string(config.network_bindip, err);

		if (err)
		{
			ShowFatalError("%s\n", err.message().c_str());
			return;
		}

		server = new tcp_server(*io_service, (address)bindip, config.network_bindport);
		server->set_default_parser(CharServer::parse_from_client);

		ShowStatus("CharServer is ready and listening on %s:%d.\n", config.network_bindip.c_str(), config.network_bindport);
	}

	// Run IO service service and start pooling events
	io_service->run();
}

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	core_display_title();

	CharServer::run();

	getchar();
	return 0;
}

/*==============================================================*
* Function:     Set a character to offline                      *    
* Author: GreenBox                                              *
* Date: 08/05/11                                                *
* Description:                                                  *	
**==============================================================*/
void CharServer::set_char_offline(int account_id, char char_id)
{
	if (online_chars.count(account_id))
	{
		if (online_chars[account_id].server > -1)
		{
			// TODO: Decrement ZoneServer users online
		}

		if (online_chars[account_id].disconnect_timer)
			TimerManager::FreeTimer(online_chars[account_id].disconnect_timer);

		if (online_chars[account_id].char_id == char_id)
		{
			online_chars[account_id].char_id = -1;
			online_chars[account_id].server = -1;
		}
	}

	if (auth_conn_ok && (char_id == -1 || !online_chars.count(account_id) || online_chars[account_id].cl->flags.eof))
	{
		WFIFOHEAD(auth_conn,6);
		WFIFOW(auth_conn,0) = INTER_CA_SET_ACC_OFF;
		WFIFOL(auth_conn,2) = account_id;
		auth_conn->send_buffer(6);
	}
}
