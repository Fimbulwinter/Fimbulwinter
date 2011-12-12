/*=========================================================*
*	______ _           _           _           _		   *
*	|  ___(_)         | |         | |         | |		   *
*	| |_   _ _ __ ___ | |__  _   _| |_   _____| |_ _ __    *
*	|  _| | | '_ ` _ \| '_ \| | | | \ \ / / _ \ __| '__|   *
*	| |   | | | | | | | |_) | |_| | |\ V /  __/ |_| |      *
*	\_|   |_|_| |_| |_|_.__/ \__,_|_| \_/ \___|\__|_|      *
* -------------------------------------------------------- *
*               An Ragnarok Online Emulator                *
* -------------------------------------------------------- *
*                Licenced under GNU GPL v3                 *
* -------------------------------------------------------- *
*              Configuration Files Reader/Parser		   *
* ======================================================== */

#include "config_file.hpp"

config_file::config_file(string filename, string delimiter, string comment, string sentry)
	: delimiter_(delimiter), comment_(comment), sentry_(sentry)
{
	std::ifstream in(filename.c_str());

	if(!in) throw file_not_found(filename); 

	in >> (*this);
}

config_file::config_file()
	: delimiter_(string(1,':')), comment_(string(1,'#'))
{
}

void config_file::remove(const string& key)
{
	contents_.erase(contents_.find(key));
	return;
}

bool config_file::key_exists(const string& key) const
{
	mapci p = contents_.find(key);
	return (p != contents_.end());
}

void config_file::trim(string& s)
{
	static const char whitespace[] = " \n\t\v\r\f";
	s.erase(0, s.find_first_not_of(whitespace));
	s.erase(s.find_last_not_of(whitespace) + 1U);
}

std::ostream& operator<<(std::ostream& os, const config_file& cf)
{
	for(config_file::mapci p = cf.contents_.begin();
		p != cf.contents_.end();
		++p)
	{
		os << p->first << " " << cf.delimiter_ << " ";
		os << p->second << std::endl;
	}
	return os;
}

std::istream& operator>>(std::istream& is, config_file& cf)
{
	typedef string::size_type pos;
	const string& delim  = cf.delimiter_;
	const string& comm   = cf.comment_;
	const string& sentry = cf.sentry_;
	const pos skip = delim.length();

	string nextline = "";

	while(is || nextline.length() > 0)
	{
		string line;
		if(nextline.length() > 0)
		{
			line = nextline;
			nextline = "";
		}
		else
		{
			std::getline(is, line);
		}

		line = line.substr(0, line.find(comm));

		if(sentry != "" && line.find(sentry) != string::npos) return is;

		pos delimPos = line.find(delim);
		if(delimPos < string::npos)
		{
			string key = line.substr(0, delimPos);
			line.replace(0, delimPos+skip, "");

			bool terminate = false;
			while(!terminate && is)
			{
				std::getline(is, nextline);
				terminate = true;

				string nlcopy = nextline;
				config_file::trim(nlcopy);
				if(nlcopy == "") continue;

				nextline = nextline.substr(0, nextline.find(comm));
				if(nextline.find(delim) != string::npos)
					continue;
				if(sentry != "" && nextline.find(sentry) != string::npos)
					continue;

				nlcopy = nextline;
				config_file::trim(nlcopy);
				if(nlcopy != "") line += "\n";
				line += nextline;
				terminate = false;
			}

			config_file::trim(key);
			config_file::trim(line);
			cf.contents_[key] = line;
		}
	}

	return is;
}
