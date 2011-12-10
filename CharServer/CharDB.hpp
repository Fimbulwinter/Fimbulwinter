#pragma once

#include <ragnarok.hpp>
#include <soci/soci.h>
#include <database_helper.h>

using namespace soci;

struct CharSessionData;
class CharDB
{
public:
	CharDB(soci::session *db)
	{
		db_ = db;
	}

	int get_char_count(int accid) 
	{
		soci::statement s =	(db_->prepare << "SELECT `char_id` FROM `char` WHERE `account_id`=:id", use(accid));
	
		s.execute(false);

		return s.get_affected_rows();
	}

	int load_chars_to_buf(int account_id, char* buf, CharSessionData *csd) 
	{
		CharData c;
		string lastmap;

		soci::statement s = (db_->prepare << "SELECT "
			"`char_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
			"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
			"`status_point`,`skill_point`,`option`,`karma`,`manner`,`hair`,`hair_color`,"
			"`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`rename`,`delete_date`,"
			"`robe`"
			" FROM `char` WHERE `account_id`=:acc AND `char_num` < :max",
			use(account_id), use(MAX_CHARS),
			into(c.char_id),
			into(c.slot),
			into(c.name),
			into(c.class_),
			into(c.base_level),
			into(c.job_level),
			into(c.base_exp),
			into(c.job_exp),
			into(c.zeny),
			into(c.str),
			into(c.agi),
			into(c.vit),
			into(c.int_),
			into(c.dex),
			into(c.luk),
			into(c.max_hp),
			into(c.hp),
			into(c.max_sp),
			into(c.sp),
			into(c.status_point),
			into(c.skill_point),
			into(c.option),
			into(c.karma),
			into(c.manner),
			into(c.hair),
			into(c.hair_color),
			into(c.clothes_color),
			into(c.weapon),
			into(c.shield),
			into(c.head_top),
			into(c.head_mid),
			into(c.head_bottom),
			into(lastmap),
			into(c.rename),
			into(c.delete_date),
			into(c.robe));

		s.execute(false);
		int i = 0, j = 0;
		while(s.fetch())
		{
			// TODO: Translate lastmap with map index

			csd->found_char[i++] = c.char_id;
			j += CharServer::char_to_buf(WBUFP(buf, j), &c);
		}

		for(; i < MAX_CHARS; i++)
			csd->found_char[i] = -1;

		return j;
	}

	void load_char(int id, CharData &c, bool full) 
	{
		string lastmap;

		soci::statement s = (db_->prepare << "SELECT "
			"`char_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
			"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
			"`status_point`,`skill_point`,`option`,`karma`,`manner`,`hair`,`hair_color`,"
			"`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`rename`,`delete_date`,"
			"`robe`"
			" FROM `char` WHERE `account_id`=:ch",
			use(id), use(MAX_CHARS),
			into(c.char_id),
			into(c.slot),
			into(c.name),
			into(c.class_),
			into(c.base_level),
			into(c.job_level),
			into(c.base_exp),
			into(c.job_exp),
			into(c.zeny),
			into(c.str),
			into(c.agi),
			into(c.vit),
			into(c.int_),
			into(c.dex),
			into(c.luk),
			into(c.max_hp),
			into(c.hp),
			into(c.max_sp),
			into(c.sp),
			into(c.status_point),
			into(c.skill_point),
			into(c.option),
			into(c.karma),
			into(c.manner),
			into(c.hair),
			into(c.hair_color),
			into(c.clothes_color),
			into(c.weapon),
			into(c.shield),
			into(c.head_top),
			into(c.head_mid),
			into(c.head_bottom),
			into(lastmap),
			into(c.rename),
			into(c.delete_date),
			into(c.robe));

		// TODO: Translate lastmap with map index

		if (full)
		{

		}
	}

	int save(CharData &c, bool nw) 
	{
		if (nw)
		{
			*db_ << "INSERT INTO `char` (`account_id`, `char_num`, `name`, `zeny`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `max_hp`, `hp`,"
			"`max_sp`, `sp`, `hair`, `hair_color`, `last_map`, `last_x`, `last_y`, `save_map`, `save_x`, `save_y`) VALUES ("
			":q, :w, :e, :r,  :t, :y, :u, :o, :p, :a, :b, :c,:d, :e,:f, :g, :h, :i, :j, :l, :m, :n)",
			use(c.account_id), use(c.slot), use(c.name), use(0 /* TODO: Start Zeny */), use(c.str), use(c.agi), use(c.vit), use(c.int_), use(c.dex), use(c.luk),
			use((40 * (100 + c.vit)/100)), use((40 * (100 + c.vit)/100 )),  use((11 * (100 + c.int_)/100)), use((11 * (100 + c.int_)/100)), use(c.hair), use(c.hair_color),
			use(string("prontera.gat") /* mapindex_id2name(start_point.map) */), use(150 /* start_point.x */), use(150 /* start_point.y */), use(string("prontera.gat") /* c.mapindex_id2name(start_point.map) */), use(150 /* start_point.x */), use(150 /* start_point.y */);

			return database_helper::get_last_insert_id(db_);
		}
		else
		{
			return c.char_id;
		}
	}

	bool delete_char(int cid) 
	{
		// TODO: Divorce, De-addopt, leave party, delete pets, delete homunculus, mercenary data, friend list, saved hotkeys, invetory, memo areas, char registry, skills, sc's and leave or break guild
		
		statement s = (db_->prepare << "DELETE FROM `char` WHERE `char_id`=:d", use(cid));
		s.execute(false);

		return true;
	}

private:
	soci::session *db_;
};
