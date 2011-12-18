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
*						Licenced under GNU GPL v3					*
* ------------------------------------------------------------------*
*						    Database Classes						*
* ================================================================= */

#pragma once

#include <string>
#include <map>
#include <datastruct.hpp>
#include <fstream>
#include <show_message.hpp>
#include <ctype.h>
#include <ragnarok.hpp>

using namespace std;

struct map_index_node
{
	int id;
	char server_name[MAP_NAME_LENGTH];
	char subname[MAP_NAME_LENGTH];
};

class map_index
{
public:
	map_index()
	{
		last_map = 1;
	}

	bool load(string file)
	{
		ifstream ifs(file, ifstream::in);
		int count = 0, line = 0, qcp = 0,j = 0;

		if (ifs.fail())
		{
			ShowError("Error reading map_index, file not found.\n");
			return false;
		}

		while (ifs.good())
		{
			char buff[256];
			char *sbuff = buff;
			line++;

			ifs.getline(buff, sizeof(buff));

			while (isspace(*sbuff) && *sbuff)
				sbuff++;

			if (buff[0] == 0)
				continue;

			if (buff[0] == '#')
				continue;

			char server_name[MAP_NAME_LENGTH];
			char copymapname[1024];
			int id = -1;
			

			if (sscanf(sbuff, "%11s %d %s",server_name,&id,copymapname) == 3 ||
				sscanf(sbuff, "%11s", server_name) == 1 ||
				sscanf(sbuff, "%11s %d", server_name, &id) == 2 
			)
				
			{

				 map_index_node *node = new map_index_node();

				 memset(node->subname, 0, sizeof(node->subname));
				 
				 if(strcmp(copymapname,"") != 0){
					 char* ptr;
					 int j = 0;
					 ptr = strtok (copymapname," ");
					   while (ptr != NULL)
					   {
						strncpy(node->subname, copymapname, sizeof(node->subname));
						ptr = strtok (NULL," ");
						j++;qcp++;
					   }
						 memset(copymapname,0,1024);
				}
				
				if (id == -1)
					id = last_map++;
				else
					last_map = id + 1;

				
				strncpy(node->server_name, server_name, sizeof(node->server_name));
				node->id = id;
				
				map_id.Insert(string(server_name), node);
				id_map[id] = node;

				count++;
			}
			else
			{
				ShowWarning("Error reading map_index at line %d.\n", line);
			}
		}

		ShowStatus("Finished reading map_index, %d maps found, %d virtual maps found.\n", count,qcp);

		ifs.close();

		return true;
	}

	int get_map_id(string server_name, bool ext = false)
	{
		if (ext)
			server_name.resize(server_name.length() - 4);

		PatriciaTrieNode<string, map_index_node *> *node = map_id.LookupNode(server_name);

		if (!node)
			throw "Map not found.";

		return node->GetData()->id;
	}

	char *get_map_name(int id)
	{
		if (!id_map.count(id))
			throw "Map not found.";

		return id_map[id]->server_name;
	}

	void copy_map_name_ext(char *dst, int id)
	{
		char tmp[MAP_NAME_LENGTH_EXT];

		if (!id_map.count(id))
			throw "Map not found.";

		if (strcmp(id_map[id]->subname, "") != 0)
			strncpy(tmp, id_map[id]->server_name, MAP_NAME_LENGTH);
		else
			strncpy(tmp, id_map[id]->subname, MAP_NAME_LENGTH);
		strcat(tmp, ".gat");
		strncpy(dst, tmp, MAP_NAME_LENGTH_EXT);
	}

private:
	PatriciaTrie<string, map_index_node *> map_id;
	map<int, map_index_node *> id_map;
	int last_map;
};
