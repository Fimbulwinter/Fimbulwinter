#pragma once

#include  "../Common/show_message.hpp"
#include  "../Common/database_helper.h"
#include  "../Common/ragnarok.hpp"
#include  "../Common/datastruct.hpp"
#include  "../Common/timers.hpp"
#include  "../Common/strfuncs.hpp"
#include  "../Common/config_file.hpp"

#include <soci/soci.h>

#include "item_db.hpp"

using namespace soci;

/// ### Import ###
PatriciaTrie<int, item_data*> ItemDB::itemtree;
item_data ItemDB::dummy;
item_data* ItemDB::itemdb_array[MAX_ITEMDB];
soci::session *ItemDB::db_;

config_file* ItemDB::item_config;
struct ItemDB::idb_config ItemDB::config;

soci::session *ItemDB::database;
ItemDB *ItemDB::items;

void ItemDB::Initialize(void)
{
	try{

		ItemDB::item_config = new config_file("./Config/item.conf");
		{
			
			ItemDB::config.idbname = item_config->read<string>("itemdb.name","itemdb");

		}

	}catch(config_file::file_not_found *fnf)
	{
		ShowFatalError("Config file not found: %s.\n", fnf->filename);
		return;
	}

	try
	{
		database = database_helper::get_session(item_config);
	}
	catch (soci::soci_error err)
	{
		ShowFatalError("Error opening database connection: %s\n", err.what());
		return;
	}

	items = new ItemDB(database);
	ShowSQL("Successfully opened item database connection.\n");
	itemdb_read();
	
}

void ItemDB::create_dummy_item(void)
{
	memset(&dummy, 0, sizeof(struct item_data));
    dummy.nameid=500;
    dummy.weight=1;
    dummy.value_sell=1;
    dummy.type=IT_ETC; 
    strncpy(dummy.name,"UNKNOWN_ITEM",sizeof(dummy.name));
    dummy.view_id=501;

}

item_data* ItemDB::create_item_data(int nameid)
{
	struct item_data *id;
	id = (struct item_data *)malloc(sizeof(struct item_data));
	id->nameid = nameid;
	id->weight = 1;
	id->type = IT_ETC;
	return id;
}

item_data* ItemDB::idbload(int itemid){

	item_data *id = new item_data();
	
	if( itemid >= 0 && itemid < GetArrayLength(itemdb_array) )
	{
		id = itemdb_array[itemid];

		if( id == NULL || id == &dummy )
			id = itemdb_array[itemid] = create_item_data(itemid);
			return id;

    }
   
	PatriciaTrieNode<int, item_data*> *node = itemtree.LookupNode(itemid);

	if(!node)
	{
		id = create_item_data(itemid);
		node->SetData(id);

	}

	return id;

}

bool ItemDB::itemdb_read_ack( vector<string> str )
{

  int itemid;
  struct item_data *id;
   
  itemid = atoi( str.at(1).c_str() );

  if( itemid <= 0 )
  {
    ShowWarning("itemdb_parse_dbrow: Invalid item id '%i', skipping\n", itemid);
    return false;
  }

  id = idbload(itemid);
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



int ItemDB::itemdb_read(void)
{
	row r;
	int count = 0;
	vector<string> str;
	vector<string>::iterator it;

	soci::statement s = (db_->prepare << "SELECT * FROM itemdb",into(r));
	s.execute(false);
	
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
	return 0;
}
