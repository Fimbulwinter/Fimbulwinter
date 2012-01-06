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

#include <boost/lexical_cast.hpp>
#include <vector>
#include <config_file.hpp>
#include <show_message.hpp>
#include <database_helper.h>
#include <ragnarok.hpp>
#include <timers.hpp>
#include <iostream>
#include <strfuncs.hpp>
#include <soci/soci.h>

using namespace soci;

/// DBOpts
#define MAX_ITEMDB 0x8000
#define MAX_SLOTS 4

static struct item_data* itemdb_array[MAX_ITEMDB];

enum item_types {
	IT_HEALING = 0,
	IT_UNKNOWN,
	IT_USABLE,  
	IT_ETC,     
	IT_WEAPON,  
	IT_ARMOR,   
	IT_CARD,    
	IT_PETEGG,  
	IT_PETARMOR,
	IT_UNKNOWN2,
	IT_AMMO,    
	IT_DELAYCONSUME,
	IT_THROWWEAPON= 17,
	IT_CASH = 18,
	IT_MAX 
};

struct item_data {
	
	    int nameid;
		char name[ITEM_NAME_LENGTH];
		int value_buy;
		int value_sell;
		int type;
		int maxchance; 
		int sex;
		int equip;
		int weight;
		int atk;
		int matk;
		int def;
		int range;
		int slot;
		int look;
		int elv;
		int wlv;
		int view_id;
		int delay;
		int class_base; /// Just to test
		int class_upper;
		/// TODO
		//unsigned int class_base[3];	
		//unsigned class_upper : 4;

		struct {
			unsigned available : 1;
			short no_equip;
			unsigned no_refine : 1;	
			unsigned delay_consume : 1;	
			unsigned trade_restriction : 7;
			unsigned autoequip: 1;
			unsigned buyingstore : 1;
		} flag;
};

class ItemDB{

public:
	
	ItemDB(soci::session *db){
		db_ = db;
	}
	
	~ItemDB();

	struct item_db_config {
		string itemdb_name;
	};

	static void Initialize();
	static struct item_data dummy_item;

	static void create_dummy_data(void)
	{
		memset(&dummy_item, 0, sizeof(struct item_data));
		dummy_item.nameid=500;
		dummy_item.weight=1;
		dummy_item.value_sell=1;
		dummy_item.type=IT_ETC; 
		strncpy(dummy_item.name,"UNKNOWN_ITEM",sizeof(dummy_item.name));
		dummy_item.view_id=501;
	}

	static struct item_data* create_item_data(int nameid)
	{
		struct item_data *id;
		id = (struct item_data *)malloc(sizeof(struct item_data));
		id->nameid = nameid;
		id->weight = 1;
		id->type = IT_ETC;
		return id;
	}

	static struct item_data* itemdb_searchname(const char *str)
	{
		struct item_data* item;
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
	
	static struct item_data* itemdb_load(int item_id)
	{
		struct item_data *id;

		if( item_id >= 0 && item_id < GetArrayLength(itemdb_array) )
		{
			id = itemdb_array[item_id];
			if( id == NULL || id == &dummy_item )
				id = itemdb_array[item_id] = create_item_data(item_id);
			return id;
		}

		PatriciaTrieNode<int, item_data*> *node = itemid.LookupNode(item_id);
	
		if(!node)
		{
			id = create_item_data(item_id);
			node->SetData(id);
		}
		
		return id;
	}

	static struct item_data* itemdb_search(int item_id)
	{
		struct item_data* id;
		
		if( item_id >= 0 && item_id < GetArrayLength(itemdb_array) ){
			id = itemdb_array[item_id];
		    return id;
		}
		
		PatriciaTrieNode<int, item_data*> *node = itemid.LookupNode(item_id);
        if(node == NULL)
		{
			ShowWarning("itemdb_search: Item ID %d does not exists in the item_db. Using dummy data.\n", item_id);
			id = &dummy_item;
			dummy_item.nameid = item_id;
		}
		
		return id;
	}

	static bool itemdb_read_ack( vector<string> str )
	{
		int itemid;
		struct item_data* id;
	
		
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

	static int itemdb_read(void)
	{
		row r;
		int count = 0;
		vector<string> str;
		vector<string>::iterator it;

		soci::statement s = (db_->prepare << "SELECT * FROM :dbname ",into(r),use(config.itemdb_name));
		
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

	static struct item_data* itemdb_exists(int id);
	static void itemdb_reload(void);

	static config_file *item_config;
	static struct item_db_config config;
	

private:
	PatriciaTrie<int, item_data *> itemid;
	soci::session *db_;


};