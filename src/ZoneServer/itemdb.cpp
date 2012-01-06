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

static struct item_data* itemdb_array[MAX_ITEMDB];
struct item_data dummy_item;

soci::session *db_;
PatriciaTrie<int, item_data*> id_item;

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

void create_dummy_data(void)
{
		memset(&dummy_item, 0, sizeof(struct item_data));
		dummy_item.nameid=500;
		dummy_item.weight=1;
		dummy_item.value_sell=1;
		dummy_item.type=IT_ETC; 
		strncpy(dummy_item.name,"UNKNOWN_ITEM",sizeof(dummy_item.name));
		dummy_item.view_id=501;
}

struct item_data* create_item_data(int nameid)
{
	struct item_data *id;
	id = (struct item_data *)malloc(sizeof(struct item_data));
	id->nameid = nameid;
	id->weight = 1;
	id->type = IT_ETC;
	return id;
}

struct item_data* itemdb_searchname(const char *str)
{
	item_data *item = new item_data();
	int i;

	for( i = 0; i < GetArrayLength(itemdb_array); ++i )
	{
		item = itemdb_array[i];
		if( item == NULL )
			continue;
	
		if( strcmp(item->name,str) == 0 )
			return item;

	}

	
}

struct item_data* itemdb_load(int itemid)
{
	item_data *id = new item_data();

	if( itemid >= 0 && itemid < GetArrayLength(itemdb_array) )
	{
		id = itemdb_array[itemid];
		if( id == NULL || id == &dummy_item )
			id = itemdb_array[itemid] = create_item_data(itemid);
			return id;
		}

	PatriciaTrieNode<int, item_data*> *node = id_item.LookupNode(itemid);
		
	if(!node)
	{
		id = create_item_data(itemid);
		node->SetData(id);
	}
		
	return id;
}

struct item_data* itemdb_search(int item_id)
{
	item_data *id = new item_data();
		
	if( item_id >= 0 && item_id < GetArrayLength(itemdb_array) ){
		id = itemdb_array[item_id];
	    return id;
	}
		
	PatriciaTrieNode<int, item_data*> *node = id_item.LookupNode(item_id);
        
	if(node == NULL)
	{
		ShowWarning("itemdb_search: Item ID %d does not exists in the item_db. Using dummy data.\n", item_id);
		id = &dummy_item;
		dummy_item.nameid = item_id;
	}
		
		return id;
}

bool itemdb_read_ack( vector<string> str )
{
	int itemid;
	item_data *id = new item_data();

		
	itemid = atoi( str.at(1).c_str() );
		
	if( itemid <= 0 )
	{
		ShowWarning("itemdb_parse_dbrow: Invalid item id '%i', skipping\n", itemid);
		return false;
	}

	id = itemdb_load(itemid);
	string name = str.at(1);

	strncpy(id->name, name.c_str() , sizeof(id->name));

	itemid = atoi( str.at(2).c_str() );

	if( id->type < 0 || id->type == IT_UNKNOWN || id->type == IT_UNKNOWN2 || ( id->type > IT_DELAYCONSUME && id->type < IT_THROWWEAPON ) || id->type >= IT_MAX )
	{
		ShowWarning("itemdb_read_ack: Invalid item type %d for item %d. IT_ETC will be used.\n", id->type, itemid);
		id->type = IT_ETC;
	}

	id->value_buy = atoi( str.at(3).c_str() );
	id->value_sell = atoi( str.at(4).c_str() );

	if( id->value_buy < id->value_sell ){
		ShowWarning("itemdb_read_ack: Zeny Exploit ( Buy Value '%i'z is less than sell value '%i'z ) Setting buy value to double sell value",id->value_buy,id->value_sell);
		id->value_buy = id->value_sell * 2;
	}

	id->weight = atoi(str.at(5).c_str() );
	id->atk = atoi(str.at(6).c_str() );
	id->matk = atoi(str.at(7).c_str() );
	id->def = atoi(str.at(8).c_str() );
	id->range = atoi(str.at(9).c_str() );
	id->slot = atoi(str.at(10).c_str() );

	if (id->slot > MAX_SLOTS)
	{
		ShowWarning("itemdb_parse_dbrow: Item %d (%s) specifies %d slots, but the server only supports up to %d.\n", itemid, id->name, id->slot, MAX_SLOTS);
		id->slot = MAX_SLOTS;
	}
		

	/// TODO
	id->class_base = atoi(str.at(11).c_str() );
	id->class_upper = atoi(str.at(12).c_str() );
	id->sex =  atoi(str.at(13).c_str() );
	id->wlv = atoi(str.at(15).c_str() );
	id->elv = atoi(str.at(16).c_str() );
	id->look =  atoi(str.at(17).c_str() );
	id->flag.no_refine =  atoi(str.at(18).c_str() );
	id->view_id =  atoi(str.at(19).c_str() );
	id->flag.available = 1;
	return true;

}

int itemdb_read(void)
{
	row r;
	int count = 0;
	vector<string> str;
	vector<string>::iterator it;

	soci::statement s = (db_->prepare << "SELECT * FROM :dbname ",into(r),use(ItemDB::config.itemdb_name));
		
	while(s.fetch()){
		for( it = str.begin(); it < str.end(); it++ )
		{
	 		 str.insert(it, r.get<string>(*it) );
			 if (!itemdb_read_ack(str))
					continue;
		}

		count++;
		ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"Item DB"CL_RESET"'.\n", count);
	}

	s.execute(false);
	return 0;

}

ItemDB::ItemDB(soci::session *db){
		db_ = db;
}
