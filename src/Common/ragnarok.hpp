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
*                        General Char Data 	               	        *
* ==================================================================*/

#pragma once

#include <time.h>
#include <string>

// Protocol Version (ZC/CZ packets)

// Note: Unlike other emulators, this setting affects both received

// and sent packets, so this needs to be configured to the version 

// of the client used, and not only to the last version which affects

// packets sent by the server. Wrong values may cause the client to

// crash, disconnect or behave unexpectedly.
#define PACKETVER 20110111

// Uncomment the folowing line if you're using one of the following

// clients which conflict with RagexeRE packets:

// 2008-09-10aSakexe, 2008-11-13aSakexe, 2008-11-26aSakexe,

// 2008-12-10aSakexe, 2009-01-14aSakexe, 2009-02-18aSakexe,

// 2009-02-25aSakexe, 2009-03-30aSakexe, 2009-04-08aSakexe.

//#define SAKEXE_CONFLICTS

/// Lenghts
#define NAME_LENGTH (23 + 1)
#define MESSAGE_SIZE (79 + 1)
#define ITEM_NAME_LENGTH 50

/// Character Infos
#define MAX_CHARS_SLOTS 9
#define MAX_CHARS MAX_CHARS_SLOTS

/// Movement Infos
#define DEFAULT_WALK_SPEED 150
#define MIN_WALK_SPEED 0
#define MAX_WALK_SPEED 1000

/// Map Infos
#define MAP_NAME_LENGTH (11 + 1)
#define MAP_NAME_LENGTH_EXT (MAP_NAME_LENGTH + 4)
#define MAP_PRONTERA "prontera"

/// Reg Infos
#define GLOBAL_REG_NUM 256
#define ACCOUNT_REG_NUM 64
#define ACCOUNT_REG2_NUM 16
#define MAX_REG_NUM 256

using namespace std;

///! \brief Map Point
struct Point 
{
	unsigned short map;
	short x,y;
};

/*! 
 *  \brief     Character Data
 *  \details   Main Character Informations
 *  \author    Fimbulwinter Development Team
 *  \author    GreenBox
 *  \date      ??/12/11
 *
 **/
struct CharData
{
	int char_id;
	int account_id;
	int partner_id;
	int father;
	int mother;
	int child;

	unsigned int base_exp,job_exp;
	int zeny;

	short class_;
	unsigned int status_point,skill_point;
	int hp,max_hp,sp,max_sp;
	unsigned int option;
	short manner;
	unsigned char karma;
	short hair,hair_color,clothes_color;
	int party_id,guild_id,pet_id,hom_id,mer_id,ele_id;
	int fame;

	// Mercenary Guilds Rank
	int arch_faith, arch_calls;
	int spear_faith, spear_calls;
	int sword_faith, sword_calls;

	short weapon; // enum weapon_type
	short shield; // view-id
	short head_top,head_mid,head_bottom;
	short robe;

	char name[NAME_LENGTH];
	unsigned int base_level,job_level;
	short str,agi,vit,int_,dex,luk;
	unsigned char slot,sex;

	unsigned int mapip;
	unsigned short mapport;

	struct Point last_point, save_point;/*,memo_point[MAX_MEMOPOINTS];
	struct item inventory[MAX_INVENTORY],cart[MAX_CART];
	struct storage_data storage;
	struct s_skill skill[MAX_SKILL];

	struct s_friend friends[MAX_FRIENDS]; //New friend system [Skotlex]
#ifdef HOTKEY_SAVING
	struct hotkey hotkeys[MAX_HOTKEYS];
#endif*/
	bool show_equip;
	short rename;

	time_t delete_date;
};

///! \brief Global Reg Table Values
struct GlobalReg
{
	char str[32];
	char value[256];
};

///! \brief Account Reg Table Values
struct AccountReg
{
	int account_id, char_id;
	int reg_num;
	struct GlobalReg reg[MAX_REG_NUM];
};

///! \brief Registry
struct Registry 
{
	int global_num;
	struct GlobalReg global[GLOBAL_REG_NUM];
	int account_num;
	struct GlobalReg account[ACCOUNT_REG_NUM];
	int account2_num;
	struct GlobalReg account2[ACCOUNT_REG2_NUM];
};

///! \brief Sex Type
enum {
	SEX_FEMALE = 0,
	SEX_MALE,
	SEX_SERVER
};

enum
{
	INTER_CA_LOGIN = 0x3000,
	INTER_AC_LOGIN_REPLY,

	INTER_AC_KICK,

	INTER_CA_AUTH,
	INTER_AC_AUTH_REPLY,

	INTER_CA_REQ_ACC_DATA,
	INTER_AC_REQ_ACC_DATA_REPLY,

	INTER_CA_SET_ACC_OFF,

	INTER_ZC_LOGIN,
	INTER_CZ_LOGIN_REPLY,

	INTER_ZC_MAPS,

	INTER_ZC_AUTH,
	INTER_CZ_AUTH_OK,
	INTER_CZ_AUTH_FAIL,

	INTER_ZC_REQ_REGS,
	INTER_ZC_REQ_REGS_REPLY,
};

#define sex_num2str(num) ( (num ==  SEX_FEMALE  ) ? 'F' : (num ==  SEX_MALE  ) ? 'M' : 'S' )
#define sex_str2num(str) ( (str == 'F' ) ?  SEX_FEMALE  : (str == 'M' ) ?  SEX_MALE  :  SEX_SERVER  )