#include "AuthServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>

#include <iostream>

// Config
config_file *AuthServer::auth_config;
config_file *AuthServer::database_config;

struct AuthServer::login_config AuthServer::config;

// Network
tcp_server *AuthServer::server;

// Database
soci::session *AuthServer::database;
AccountDB *AuthServer::accounts;

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

	// Initialize Database System
	{
		ShowInfo("Opening connection to database...\n");

		database = database_helper::get_session(database_config);
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

int AuthServer::parse_from_client(tcp_connection::pointer cl)
{
	if (cl->flags.eof)
	{
		cl->do_close();

		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{
		case 0x0064:



			break;
		default:
			ShowWarning("Unknown packet 0x%x sent from %s:%d, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str(), cl->socket().remote_endpoint().port());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_YELLOW"       Equipe Cronus de Desenvolvimento Apresenta        "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      _________                                          "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      \\_   ___ \\_______  ____   ____  __ __  ______      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      /    \\  \\/\\_  __ \\/  _ \\ /    \\|  |  \\/  ___/      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      \\     \\____|  | \\(  <_> )   |  \\  |  /\\___ \\       "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"       \\______  /|__|   \\____/|___|  /____//____  >      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"              \\/                   \\/           \\/       "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                         "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_RED"                         Cronus++                        "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                  www.cronus-emulator.com                "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                         "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n\n");

	AuthServer::run();

	getchar();
	return 0;
}
