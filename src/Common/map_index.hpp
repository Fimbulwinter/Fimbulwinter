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
	char name[MAP_NAME_LENGTH];
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
		int count = 0, line = 0;

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

			char name[MAP_NAME_LENGTH];
			int id = -1;
			
			if (sscanf(sbuff, "%10s", name) == 1 ||
				sscanf(sbuff, "%10s %d", name, &id) == 2)
			{
				if (id == -1)
					id = last_map++;
				else
					last_map = id + 1;

				map_index_node *node = new map_index_node();
				strncpy(node->name, name, sizeof(node->name));
				node->id = id;
				
				map_id.Insert(string(name), node);
				id_map[id] = node;

				count++;
			}
			else
			{
				ShowWarning("Error reading map_index at line %d.\n", line);
			}
		}

		ShowStatus("Finished reading map_index, %d maps found.", count);

		ifs.close();

		return true;
	}

	int get_map_id(string name)
	{
		PatriciaTrieNode<string, map_index_node *> *node = map_id.LookupNode(name);

		if (!node)
			throw "Map not found.";

		return node->GetData()->id;
	}

	char *get_map_name(int id)
	{
		if (!id_map.count(id))
			throw "Map not found.";

		return id_map[id]->name;
	}

private:
	PatriciaTrie<string, map_index_node *> map_id;
	map<int, map_index_node *> id_map;
	int last_map;
};
