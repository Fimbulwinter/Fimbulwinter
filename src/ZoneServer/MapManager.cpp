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
*                           Map Manager	               	            *
* ==================================================================*/

#include "MapManager.hpp"
#include "ZoneServer.hpp"
#include <zlib.h>
#include <fstream>

vector<MapData> MapManager::maps;
map_index MapManager::mapsidx;

/*! 
 *  \brief     Map Gat to Cell
 *  \details   Reads a gat and convert the information to cells.
 *  \author    Fimbulwinter Development Team
 *  \author    Vianna
 *  \date      28/12/11
 *
 *	@param	gat	0 - Walkable and shootable floor 
 *	@param	gat	1 - Non-Walkable and non shootable floor 
 *	@param	gat	2 - ???
 *	@param	gat	3 - Walkable and shootable water
 *	@param	gat	4 - ???
 *	@param	gat 5 - Non-Walkable and shootable floor
 *	@param	gat	6 - ???
 **/
inline static struct MapCell map_gat2cell(int gat)
{
	struct MapCell cell = { 0 };

    if ((gat < 0) || (gat > 6))
		ShowWarning("map_gat2cell: unrecognized gat type '%d'\n", gat);
	else
	{
		if (gat == 1 || gat == 5) cell.walkable = 0;
		else cell.walkable = 1;
		if (gat == 1) cell.shootable = 0;
		else cell.shootable = 1;
		if (gat == 3) cell.water = 1;
		else cell.water = 0;
	}

	return cell;
}

/*! 
 *  \brief     Decode Zip
 *  \details   Read a zip file.
 *  \author    Fimbulwinter Development Team
 *  \author    Vianna
 *  \date      28/12/11
 *
 **/
inline static int decode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen)
{
	return uncompress((Bytef *)dest, destLen, (const Bytef *)source, sourceLen);
}

/*! 
 *  \brief     Initialize MapManager
 *  \details   Read MapCache and MapFiles
 *  \author    Fimbulwinter Development Team
 *  \author    Vianna
 *  \date      28/12/11
 *
 **/
void MapManager::initialize()
{
	vector<int> my_maps;

	// Load MapIndex
	if (!mapsidx.load("data/map_index"))
	{
		getchar();
		abort();
	}

	{
		// Loading ZoneServer map list
		ifstream ifs("config/zonemaps.lst", ifstream::in);
		int line = 0;

		if (ifs.fail())
		{
			ShowFatalError("Error reading ZoneServer map file, file not found.\n");
			abort();
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

			char *tmp = sbuff;
			while (*tmp)
			{
				if (isspace(*tmp))
				{
					*tmp = 0;
					break;
				}

				tmp++;
			}

			int m;
			try
			{
				m = mapsidx.get_map_id(string(sbuff));
			}
			catch (char *msg)
			{
				ShowWarning("Error parsing ZoneMaps file on line %d: %s\n", line, msg);
				continue;
			}

			my_maps.push_back(m);
		}

		if (my_maps.size() == 0)
		{
			ShowFatalError("No maps assigned to this server.\n");
			abort();
		}

		// Create map array
		maps.resize(my_maps.size());
		for (unsigned int i = 0; i < my_maps.size(); i++)
		{
			memset(&maps[i], 0, sizeof(struct MapData));

			maps[i].m = my_maps[i];
			strcpy(maps[i].name, mapsidx.get_map_name(maps[i].m));
		}
	}

	{
		// Read eAthena MapCache
		char *map_cache_buffer;
		int sz;
		ShowStatus("Loading map-cache...\n");

		ifstream ifs("data/map_cache", ifstream::binary | ifstream::in);

		if (ifs.fail())
		{
			ShowFatalError("Error reading map-cache, file not found.\n");
			abort();
		}

		ifs.seekg(0, ios::end);
		sz = (int)ifs.tellg();
		ifs.seekg(0, ios::beg);

		map_cache_buffer = (char*)malloc(sz);
		ifs.read(map_cache_buffer, sz);
		ifs.close();

		int maps_removed = 0;
		char map_cache_decode_buffer[MAX_MAP_SIZE];

		ShowStatus("Loading maps (%d)..\n", my_maps.size());
		for (unsigned int i = 0; i < my_maps.size(); i++)
		{
			decode_mapcache(&maps[i], map_cache_buffer, map_cache_decode_buffer);

			maps[i].wb = (maps[i].w + BLOCK_SIZE - 1) / BLOCK_SIZE;
			maps[i].hb = (maps[i].h + BLOCK_SIZE - 1) / BLOCK_SIZE;

			maps[i].blocks = new struct BlockList *[maps[i].wb * maps[i].hb];
		}

		free(map_cache_buffer);
	}
}

/*! 
 *  \brief     Decode MapCache
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    Vianna
 *  \date      28/12/11
 *
 **/
bool MapManager::decode_mapcache(struct MapData *m, char *buffer, char *decode_buffer)
{
	int i;
	struct MapCacheHeader *header = (struct MapCacheHeader *)buffer;
	struct MapCacheMapInfo *info = NULL;
	char *p = buffer + sizeof(struct MapCacheHeader);
	static char mapname[MAP_NAME_LENGTH];

	strcpy(mapname, m->name);
	sscanf(m->name, "%*[^#]%*[#]%s", mapname);

	for(i = 0; i < header->map_count; i++) 
	{
		info = (struct MapCacheMapInfo *)p;

		if(strcmp(mapname, info->name) == 0)
			break;

		p += sizeof(struct MapCacheMapInfo) + info->len;
	}

	if(info && i < header->map_count) 
	{
		unsigned long size, xy;

		if(info->xs <= 0 || info->ys <= 0)
			return false;

		m->w = info->xs;
		m->h = info->ys;
		size = (unsigned long)info->xs * (unsigned long)info->ys;

		if(size > MAX_MAP_SIZE) 
		{
			ShowWarning("(MapManager::decode_mapcache): %s exceeded MAX_MAP_SIZE of %d\n", info->name, MAX_MAP_SIZE);
			return false;
		}

		decode_zip(decode_buffer, &size, p + sizeof(struct MapCacheMapInfo), info->len);

		m->cell = (struct MapCell *)malloc(sizeof(struct MapCell) * (unsigned long)info->xs * (unsigned long)info->ys);

		for(xy = 0; xy < size; xy++)
			m->cell[xy] = map_gat2cell(decode_buffer[xy]);

		return true;
	}

	return false;
}

/*! 
 *  \brief     Check a MapCell
 *  
 *  \author    Fimbulwinter Development Team
 *  \author    Vianna
 *  \date      28/12/11
 *
 **/
bool MapManager::check_cell( unsigned int mi, short x, short y, CellCheck cellchk )
{
	struct MapData *m;
	
	if (mi < 0 || mi > maps.size())
		return false;

	m = &maps[mi];

	struct MapCell cell;

	// NOTE: this intentionally overrides the last row and column
	if(x < 0 || x >= m->w - 1 || y < 0 || y >= m->h - 1)
		return (cellchk == CELL_CHKNOPASS);

	cell = m->cell[x + y * m->w];

	switch(cellchk)
	{
	case CELL_CHKWALL:
		return (!cell.walkable && !cell.shootable);

	case CELL_CHKWATER:
		return (cell.water);

	case CELL_CHKCLIFF:
		return (!cell.walkable && cell.shootable);

	case CELL_CHKNPC:
		return (cell.npc);
	case CELL_CHKBASILICA:
		return (cell.basilica);
	case CELL_CHKLANDPROTECTOR:
		return (cell.landprotector);
#ifdef ENABLE_CUSTOM_CELLFLAGS
	case CELL_CHKNOVENDING:
		return (cell.novending);
	case CELL_CHKNOCHAT:
		return (cell.nochat);
#endif
	case CELL_CHKMAELSTROM:
		return (cell.maelstrom);

		// special checks
	case CELL_CHKPASS:
#ifdef CELL_NOSTACK
		if (cell.cell_bl >= battle_config.cell_stack_limit) return true;
#endif
	case CELL_CHKREACH:
		return (cell.walkable);

	case CELL_CHKNOPASS:
#ifdef CELL_NOSTACK
		if (cell.cell_bl >= battle_config.cell_stack_limit) return false;
#endif
	case CELL_CHKNOREACH:
		return (!cell.walkable);

	case CELL_CHKSTACK:
#ifdef CELL_NOSTACK
		return (cell.cell_bl >= battle_config.cell_stack_limit);
#else
		return false;
#endif

	default:
		return false;
	}

	return false;
}
