#pragma once

#include  "../Common/show_message.hpp"
#include  "../Common/database_helper.h"
#include  "../Common/ragnarok.hpp"
#include  "../Common/datastruct.hpp"
#include  "../Common/timers.hpp"
#include  "../Common/strfuncs.hpp"
#include  "../Common/config_file.hpp"

#include <soci/soci.h>

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

class ItemDB {

private:
	static soci::session *db_;
	static PatriciaTrie<int, item_data*> itemtree;
	static item_data dummy;
	static item_data* itemdb_array[MAX_ITEMDB];
	
public:

	ItemDB(soci::session* db)  {

		db_ = db;

	}

	struct idb_config {

		string idbname;

	};

	static soci::session *database;
	static ItemDB *items;

	static void create_dummy_item(void);
	static item_data* create_item_data(int);
	static void Initialize(void);
	static item_data* idbload(int);
	static bool itemdb_read_ack( vector<string> );
	static int itemdb_read(void);


	static config_file* item_config;
	static struct idb_config config;


};
