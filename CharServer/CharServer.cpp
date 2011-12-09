#include "CharServer.hpp"

#include <show_message.hpp>
#include <database_helper.h>
#include <boost/thread.hpp>
#include <ragnarok.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>

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

		database = database_helper::get_session(database_config);

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

void CharServer::connect_to_auth()
{
	for (;;)
	{
		ShowInfo("Connecting to AuthServer on %s:%d...\n", config.inter_login_ip.c_str(), config.inter_login_port);

		boost::system::error_code ec;

		address_v4 addr = address_v4::from_string(config.inter_login_ip, ec);

		if (ec)
		{
			ShowFatalError("%s\n", ec.message().c_str());
			abort();
		}

		tcp::endpoint ep((address)addr, config.inter_login_port);

		auth_conn = tcp_connection::create(*io_service);
		auth_conn->set_parser(&CharServer::parse_from_login);
		auth_conn->socket().connect(ep, ec);

		if (ec)
		{
			ShowError("%s\n", ec.message().c_str());
			auth_conn.reset();

			{
				boost::xtime xt;

				boost::xtime_get(&xt, boost::TIME_UTC);
				xt.sec += 5;

				boost::thread::sleep(xt);
			}

			continue;
		}

		auth_conn->start();

		WFIFOHEAD(auth_conn, 76);
		WFIFOW(auth_conn, 0) = INTER_CA_LOGIN;
		strncpy((char*)WFIFOP(auth_conn, 2), config.inter_login_user.c_str(), NAME_LENGTH);
		strncpy((char*)WFIFOP(auth_conn, 26), config.inter_login_pass.c_str(), NAME_LENGTH);
		strncpy((char*)WFIFOP(auth_conn, 50), config.server_name.c_str(), 20);
		WFIFOL(auth_conn, 70) = htonl(address_v4::from_string(config.network_charip).to_ulong());
		WFIFOW(auth_conn, 74) = htons(config.network_bindport);
		auth_conn->send_buffer(76);

		break;
	}
}

int CharServer::parse_from_zone(tcp_connection::pointer cl)
{
	return 0;
}

int CharServer::parse_from_login(tcp_connection::pointer cl)
{
	if (cl->flags.eof)
	{
		cl->do_close();

		connect_to_auth();

		return 0;
	}

	while(RFIFOREST(cl) >= 2)
	{
		SWITCH_PACKET()
		{
		case INTER_AC_LOGIN_REPLY:
			{
				unsigned char result = RFIFOB(cl, 2);

				cl->skip(3);

				if (result == 0)
				{
					ShowStatus("Connected to AuthServer.\n");
				}
				else
				{
					ShowError("Connectiong rejected from AuthServer.");
					cl->set_eof();
					return 0;
				}
			}
			break;
		default:
			ShowWarning("Unknown packet 0x%x sent from AuthServer, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

int CharServer::parse_from_client(tcp_connection::pointer cl)
{
	//AuthSessionData *asd = ((AuthSessionData *)cl->get_data());

	if (cl->flags.eof)
	{
		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", cl->socket().remote_endpoint().address().to_string().c_str());
		cl->do_close();
		return 0;
	}

	/*if (asd == NULL)
	{
		asd = new AuthSessionData();
		memset(asd, 0, sizeof(asd));

		asd->cl = cl;

		cl->set_data((char*)asd);
	}*/

	while(RFIFOREST(cl) >= 2)
	{
		unsigned short cmd = RFIFOW(cl, 0);

		switch (cmd)
		{
		default:
			ShowWarning("Unknown packet 0x%04x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
			cl->set_eof();
			return 0;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	core_display_title();

	CharServer::run();

	getchar();
	return 0;
}
