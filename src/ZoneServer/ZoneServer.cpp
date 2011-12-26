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
*                         Character Server 	               	        *
* ==================================================================*/

#include "ZoneServer.hpp"

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
#include <fstream>
#include <zlib.h>
#include "mapmanager.hpp"

static struct BlockList bl_head;

// Login InterConn
tcp_connection::pointer ZoneServer::char_conn;

// Config
config_file *ZoneServer::char_config;

struct ZoneServer::login_config ZoneServer::config;

// Network
boost::asio::io_service *ZoneServer::io_service;
tcp_server *ZoneServer::server;

// Database
soci::session *ZoneServer::database;

// Auth
ZoneServer::auth_node_db ZoneServer::auth_nodes;
ZoneServer::online_account_db ZoneServer::online_chars;
bool ZoneServer::char_conn_ok;

/*==============================================================*
* Function:	Start Char Server									*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Start the char-server and load the confs         *
**==============================================================*/
void ZoneServer::run()
{
	io_service = new boost::asio::io_service();

	// Read Config Files
	try
	{
		char_config = new config_file("./Config/ZoneServer.conf");
		{
			config.network_bindip = char_config->read<string>("network.bindip", "0.0.0.0");
			config.network_bindport = char_config->read<unsigned short>("network.bindport", 6121);

			config.network_zoneip = char_config->read<string>("network.zoneip", "");

			config.inter_char_ip = char_config->read<string>("inter.char.ip", "127.0.0.1");
			config.inter_char_port = char_config->read<unsigned short>("inter.char.port", 6900);
			config.inter_char_user = char_config->read<string>("inter.char.user", "s1");
			config.inter_char_pass = char_config->read<string>("inter.char.pass", "p1");

			if (config.network_zoneip == "")
			{
				ShowInfo("Auto-detecting my IP Address...\n");
				
				tcp::resolver resolver(*io_service);
				tcp::resolver::query query(boost::asio::ip::host_name(), "");
				tcp::resolver::iterator iter = resolver.resolve(query);
				tcp::resolver::iterator end;

				while (iter != end)
				{
					tcp::endpoint ep = *iter++;

					if (!ep.address().is_v4())
						continue;
					
					config.network_zoneip = ep.address().to_string();

					break;
				}

				ShowStatus("Defaulting our IP Address to %s...\n", config.network_zoneip.c_str());
			}
		}
		ShowStatus("Finished reading ZoneServer.conf.\n");
	}
	catch (config_file::file_not_found *fnf)
	{
		ShowFatalError("Config file not found: %s.\n", fnf->filename);
		return;
	}

	TimerManager::Initialize(io_service);
	MapManager::initialize();

	// Initialize Database System
	{
		ShowInfo("Opening connection to database...\n");

		try
		{
			database = database_helper::get_session(char_config);
		}
		catch (soci::soci_error err)
		{
			ShowFatalError("Error opening database connection: %s\n", err.what());
			return;
		}

		ShowSQL("Successfully opened database connection.\n");
	}

	init_packets();
	connect_to_char();

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
		server->set_default_parser(ZoneServer::parse_from_client);

		ShowStatus("ZoneServer is ready and listening on %s:%d.\n", config.network_bindip.c_str(), config.network_bindport);
	}

	// Run IO service service and start pooling events
	io_service->run();
}

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	core_display_title();

	ZoneServer::run();

	getchar();
	return 0;
}

/*==============================================================*
* Function:     Set a character to offline                      *    
* Author: GreenBox                                              *
* Date: 08/05/11                                                *
* Description:                                                  *	
**==============================================================*/
void ZoneServer::set_char_offline(int account_id, char char_id)
{
	
}

void ZoneServer::create_auth_entry(ZoneSessionData *sd, enum SessionState state)
{
	if (auth_nodes.count(sd->status.account_id))
		return;

	auth_nodes[sd->status.account_id].account_id = sd->status.account_id;
	auth_nodes[sd->status.account_id].char_id = sd->status.char_id;
	auth_nodes[sd->status.account_id].login_id1 = sd->login_id1;
	auth_nodes[sd->status.account_id].login_id2 = sd->login_id2;
	auth_nodes[sd->status.account_id].sex = sd->status.sex;
	auth_nodes[sd->status.account_id].cl = sd->cl;
	auth_nodes[sd->status.account_id].sd = sd;
	auth_nodes[sd->status.account_id].node_created = (unsigned int)time(NULL);
	auth_nodes[sd->status.account_id].auth_state = state;

	sd->state.active = 0;
}

void ZoneServer::addblock( struct BlockList* bl )
{
	int m, x, y, pos;

	if (!bl)
		return;

	if (bl->prev != NULL) 
	{
		ShowError("ZoneServer::addblock: bl->prev != NULL\n");
		return;
	}

	m = bl->m;
	x = bl->x;
	y = bl->y;
	if (m < 0 || m >= MapManager::maps.size())
	{
		ShowError("ZoneServer::addblock: invalid map id (%d), only %d are loaded.\n", m, MapManager::maps.size());
		return;
	}

	if (x < 0 || x >= MapManager::maps[m].w || y < 0 || y >= MapManager::maps[m].h)
	{
		ShowError("ZoneServer::addblock: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d\n", MapManager::maps[m].name, x, y, MapManager::maps[m].w, MapManager::maps[m].h);
		return;
	}

	pos = x / BLOCK_SIZE + (y / BLOCK_SIZE) * MapManager::maps[m].wb;

	if (bl->type == BL_MOB) 
	{
		/*bl->next = MapManager::maps[m].block_mob[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		map[m].block_mob[pos] = bl;*/
		// TODO: soon
	}
	else
	{
		bl->next = MapManager::maps[m].blocks[pos];
		bl->prev = &bl_head;

		if (bl->next) 
			bl->next->prev = bl;

		MapManager::maps[m].blocks[pos] = bl;
	}

#ifdef CELL_NOSTACK
	// TODO:? map_addblcell(bl);
#endif
}
