#pragma once

#include <soci/soci.h>
#include <database_helper.h>

using namespace soci;

class Account
{
public:
	int account_id;
	string userid;
	string user_pass;        // 23+1 for plaintext, 32+1 for md5-ed passwords
	char sex;               // gender (M/F/S)
	string email;         // e-mail (by default: a@a.com)
	int level;              // GM level
	unsigned int state;     // packet 0x006a value + 1 (0: compte OK)
	time_t unban_time;      // (timestamp): ban time limit of the account (0 = no ban)
	time_t expiration_time; // (timestamp): validity limit of the account (0 = unlimited)
	unsigned int logincount;// number of successful auth attempts
	string lastlogin;     // date+time of last successful login
	string last_ip;       // save of last IP of connection
	string birthdate;   // assigned birth date (format: YYYY-MM-DD, default: 0000-00-00)
};

class AccountDB
{
public:
	AccountDB(soci::session *db)
	{
		db_ = db;
	}

	bool load_account(int accid, Account &acc)
	{
		soci::statement s =	(db_->prepare << "SELECT `account_id`, `userid`, `user_pass`, `sex`, `email`, `level`, \
			`state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, \
			`last_ip`, `birthdate` FROM `login` WHERE `account_id`=:id", use(accid),
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

	bool load_account(string name, Account &acc)
	{
		soci::statement s =	(db_->prepare << "SELECT `account_id`, `userid`, `user_pass`, `sex`, `email`, `level`, \
							`state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, \
							`last_ip`, `birthdate` FROM `login` WHERE `userid`=:name", use(name),
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

	void save_account(Account &acc, bool nw = false)
	{
		if (nw)
		{
			*db_ << "INSERT INTO `login`(`account_id`, `userid`, `user_pass`, `sex`, `email`, `level`, \
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
			*db_ << "UPDATE `login` `login` SET `userid`=:uid, `user_pass`=:pw, `sex`=:s, `email`=:email, `level`=:lvl, \
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
