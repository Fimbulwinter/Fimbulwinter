/*==================================================================*
*     ___ _           _           _          _       _				*
*    / __(_)_ __ ___ | |__  _   _| |_      _(_)_ __ | |_ ___ _ __	*
*   / _\ | | '_ ` _ \| '_ \| | | | \ \ /\ / / | '_ \| __/ _ \ '__|	*
*  / /   | | | | | | | |_) | |_| | |\ V  V /| | | | | ||  __/ |		*
*  \/    |_|_| |_| |_|_.__/ \__,_|_| \_/\_/ |_|_| |_|\__\___|_|		*
*																	*
* ------------------------------------------------------------------*
*							    Emulator			                *
* ------------------------------------------------------------------*
*                        Licenced under GNU GPL v3                  *
* ----------------------------------------------------------------- *
*                      Configuration Files Modules      		    *
* ==================================================================*/

#pragma once

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using std::string;

class config_file 
{
protected:
	string delimiter_;
	string comment_;
	string sentry_;
	std::map<string,string> contents_;

	typedef std::map<string,string>::iterator mapi;
	typedef std::map<string,string>::const_iterator mapci;

public:
	config_file(string filename,
		string delimiter = "=",
		string comment = "#",
		string sentry = "EndConfigFile");
	config_file();

	template<class T> T read(const string& key) const;
	template<class T> T read(const string& key, const T& value) const;
	template<class T> bool read_into(T& var, const string& key) const;
	template<class T>
	bool read_into(T& var, const string& key, const T& value) const;

	template<class T> void add(string key, const T& value);
	void remove(const string& key);

	bool key_exists(const string& key) const;

	string get_delimiter() const { return delimiter_; }
	string get_comment() const { return comment_; }
	string get_sentry() const { return sentry_; }
	string set_delimiter(const string& s)
	{ string old = delimiter_;  delimiter_ = s;  return old; }  
	string set_comment(const string& s)
	{ string old = comment_;  comment_ = s;  return old; }

	friend std::ostream& operator<<(std::ostream& os, const config_file& cf);
	friend std::istream& operator>>(std::istream& is, config_file& cf);

protected:
	template<class T> static string T_as_string(const T& t);
	template<class T> static T string_as_T(const string& s);

public:
	static void trim(string& s);

	struct file_not_found 
	{
		string filename;
		file_not_found(const string& filename_ = string())
			: filename(filename_) {} 
	};
	struct key_not_found 
	{
		string key;
		key_not_found(const string& key_ = string())
			: key(key_) {} 
	};
};

template<class T>
string config_file::T_as_string(const T& t)
{
	std::ostringstream ost;
	ost << t;
	return ost.str();
}

template<class T>
T config_file::string_as_T(const string& s)
{
	T t;
	std::istringstream ist(s);
	ist >> t;
	return t;
}

template<>
inline string config_file::string_as_T<string>(const string& s)
{
	return s;
}

template<>
inline char *config_file::string_as_T<char*>(const string& s)
{
	return (char*)s.c_str();
}

template<>
inline bool config_file::string_as_T<bool>(const string& s)
{
	bool b = true;
	string sup = s;
	for(string::iterator p = sup.begin(); p != sup.end(); ++p)
		*p = toupper(*p);
	if(sup==string("FALSE") || sup==string("F") ||
		sup==string("NO") || sup==string("N") ||
		sup==string("0") || sup==string("NONE"))
		b = false;
	return b;
}


template<class T>
T config_file::read(const string& key) const
{
	mapci p = contents_.find(key);
	if(p == contents_.end()) throw key_not_found(key);
	return string_as_T<T>(p->second);
}


template<class T>
T config_file::read(const string& key, const T& value) const
{
	mapci p = contents_.find(key);
	if(p == contents_.end()) return value;
	return string_as_T<T>(p->second);
}


template<class T>
bool config_file::read_into(T& var, const string& key) const
{
	mapci p = contents_.find(key);
	bool found = (p != contents_.end());
	if(found) var = string_as_T<T>(p->second);
	return found;
}


template<class T>
bool config_file::read_into(T& var, const string& key, const T& value) const
{
	mapci p = contents_.find(key);
	bool found = (p != contents_.end());
	if(found)
		var = string_as_T<T>(p->second);
	else
		var = value;
	return found;
}


template<class T>
void config_file::add(string key, const T& value)
{
	string v = T_as_string(value);
	trim(key);
	trim(v);
	contents_[key] = v;
	return;
}
