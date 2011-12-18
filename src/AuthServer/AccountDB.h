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
*                        Account Databases 	               	        *
* ==================================================================*/

#pragma once

#include <soci/soci.h>
#include <database_helper.h>

using namespace soci;


/**
 * Account Class 
 * Last Update: 08/12/11
 * Author: GreenBox (Fimbulwinter Dev Team)
 *
 * @param	account_id	The Account ID
 * @param	userid	The User Login 
 * @param	user_pass	User Password (Size: 23+1 plaintext, 32+1 MD5)
 * @param	sex	User Gender 
 * @param	email	User Email
 * @param	level	GM Level
 * @param	state	Account State (packet 0x006a value + 1 (0: compte OK))
 * @param	unban_time	(timestamp): ban time limit of the account (0 = no ban)
 * @param	expiration_time (timestamp): validity limit of the account (0 = unlimited)
 * @param	logincount Number of successful login attempts
 * @param	lastlogin	Last Login date
 * @param	last_ip	Last Login IP
 * @param	birthdate	assigned birth date (format: YYYY-MM-DD, default: 0000-00-00)
 *
 * @return	void	No Return
 **/
class Account
{
public:
	int account_id;
	string userid;
	string user_pass;      
	char sex;              
	string email;         
	int level;              
	unsigned int state;     
	time_t unban_time;      
	time_t expiration_time; 
	unsigned int logincount;
	string lastlogin;     
	string last_ip;       
	string birthdate;  
};

/**
 * Account Database 
 * Last Update: 08/12/11
 * Author: GreenBox (Fimbulwinter Dev Team)
 *
 * @param	load_account	Loads the server accounts( By ID and Name )
 * @param	save_account	Saves the account into the database 
 *
 * @return	bool	Returns Success or Failure
 **/
class AccountDB
{
public:
	AccountDB(soci::session *db) // AccountDB Constructor
	{
		db_ = db;
	}

	bool load_account(int accid, Account &acc) // Load Account Module ( By ID )
	{
		soci::statement s =	(db_->prepare << "SELECT `account_id`, `userid`, `user_pass`, `sex`, `email`, `level`, \
			`state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, \
			`last_ip`, `birthdate` FROM `login` WHERE `account_id`=:id LIMIT 1", use(accid),
			into(acc.account_id),
			into(acc.userid),
			into(acc.user_pass),
			into(acc.sex),
			into(acc.email),
			into(acc.level),
			into(acc.state),
			into(acc.unban_time),
			into(acc.expiration_time),
			into(acc.logincount),
			into(acc.lastlogin),
			into(acc.last_ip),
			into(acc.birthdate));

		s.execute(true);

		return s.get_affected_rows() == 1;
	}

	bool load_account(string name, Account &acc) // Load Account Module ( By Name )
	{
		soci::statement s =	(db_->prepare << "SELECT `account_id`, `userid`, `user_pass`, `sex`, `email`, `level`, \
							`state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, \
							`last_ip`, `birthdate` FROM `login` WHERE `userid`=:name LIMIT 1", use(name),
							into(acc.account_id),
							into(acc.userid),
							into(acc.user_pass),
							into(acc.sex),
							into(acc.email),
							into(acc.level),
							into(acc.state),
							into(acc.unban_time),
							into(acc.expiration_time),
							into(acc.logincount),
							into(acc.lastlogin),
							into(acc.last_ip),
							into(acc.birthdate));
		
		s.execute(true);

		return s.get_affected_rows() == 1;
	}

	void save_account(Account &acc, bool nw = false) // Save the Account into the database.
	{
		if (nw)
		{
			*db_ << "INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `level`, \
					`state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, \
					`last_ip`, `birthdate`) VALUES (\
					:id, :uid, :pwd, :sex, :email, :lvl, :state, :unban, :expr, :loginc, \
					:lip, :birth)",
					use(acc.account_id),
					use(acc.userid),
					use(acc.user_pass),
					use(acc.sex),
					use(acc.email),
					use(acc.level),
					use(acc.state),
					use(acc.unban_time),
					use(acc.expiration_time),
					use(acc.logincount),
					use(acc.lastlogin),
					use(acc.last_ip),
					use(acc.birthdate);
		}
		else
		{
			*db_ << "UPDATE `login` SET `userid`=:uid, `user_pass`=:pw, `sex`=:s, `email`=:email, `level`=:lvl, \
					`state`=:state, `unban_time`=:unban, `expiration_time`=:expr, `logincount`=:loginc, `lastlogin`=:llogin, \
					`last_ip`=:lip, `birthdate`=:birth WHERE `account_id`=:id",
					use(acc.userid),
					use(acc.user_pass),
					use(acc.sex),
					use(acc.email),
					use(acc.level),
					use(acc.state),
					use(acc.unban_time),
					use(acc.expiration_time),
					use(acc.logincount),
					use(acc.lastlogin),
					use(acc.last_ip),
					use(acc.birthdate),
					use(acc.account_id);
		}
	}

private:
	soci::session *db_;
};
