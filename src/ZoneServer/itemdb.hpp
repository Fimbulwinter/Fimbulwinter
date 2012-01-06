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
	
	ItemDB(soci::session *db);
	
	~ItemDB();

	struct item_db_config {
		string itemdb_name;
	};


	static void Initialize();
	static void create_dummy_data(void);
	static struct item_data* create_item_data(int nameid);
	static struct item_data* itemdb_searchname(const char *str);
	static struct item_data* itemdb_load(int itemid);
	static struct item_data* itemdb_search(int item_id);
	static bool itemdb_read_ack( vector<string> str );
	static int itemdb_read(void);


	static config_file *item_config;
	static struct item_db_config config;
	

private:

};