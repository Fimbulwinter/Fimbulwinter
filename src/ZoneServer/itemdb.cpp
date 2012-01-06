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
#pragma once

#include <config_file.hpp>
#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>
#include <strfuncs.hpp>
#include <datastruct.hpp>
#include "itemdb.hpp"

/// Config File
config_file *ItemDB::item_config;
struct ItemDB::item_db_config ItemDB::config;

void ItemDB::Initialize()
{

	try
	{
		item_config = new config_file("./Config/itemdb.conf");
		{
			config.itemdb_name = item_config->read<string>("itemdb.name","itemdb");
		}

	}catch (config_file::file_not_found *fnf)
	{
		ShowFatalError("Config file not found: %s.\n", fnf->filename);
		return;
	}

	itemdb_read();

}

