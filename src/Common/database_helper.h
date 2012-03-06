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
*                        Database Functions  	               	    *
* ==================================================================*/

#pragma once

#include "config_file.hpp"
#include "show_message.hpp"
#include <soci/soci.h>

#include <cstdlib>

class database_helper
{
private:
	static std::string driver_;

public:
	static soci::session *get_session(config_file *conf)
	{
		char connection_string[1024]; // 1024 should be enought
		
		{
			string driver = conf->read<string>("database.driver", "mysql");

			if (driver == "mysql")
			{
				string host = conf->read<string>("database.mysql.host", "127.0.0.1");
				string port = conf->read<string>("database.mysql.port", "3306");
				string user = conf->read<string>("database.mysql.user", "ragnarok");
				string pass = conf->read<string>("database.mysql.pass", "ragnarok");
				string name = conf->read<string>("database.mysql.name", "ragnarok");

				sprintf(connection_string, "mysql://host='%s' port='%s' user='%s' password='%s' db='%s'", host.c_str(), port.c_str(), user.c_str(), pass.c_str(), name.c_str());
			}
			else if (driver == "sqlite3")
			{
				string file = conf->read<string>("database.sqlite3.file", "ragnarok.db");

				sprintf(connection_string, "sqlite://%s", file.c_str());
			}
			else
			{
				ShowError("Invalid driver database.");

				abort();
				
				return NULL;
			}

			driver_ = driver;
		}

		return new soci::session((std::string)connection_string);
	}

	static int get_last_insert_id(soci::session *s)
	{
		int result = 0;

		if (driver_ == "mysql")
			*s << "SELECT LAST_INSERT_ID()", soci::into(result);
		else if (driver_ == "sqlite3")
		{
			// TODO: Implement
		}

		return result;
	}
};
