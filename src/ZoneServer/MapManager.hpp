#pragma once

#include <ragnarok.hpp>
#include <map_index.hpp>
#include <vector>

#define BLOCK_SIZE 8
#define MAX_MAP_SIZE 512 * 512

// Uncomment to activate custom cellflags like novending and nochat
// Note that this will increase map cells memory usage by 100%
//#define ENABLE_CUSTOM_CELLFLAGS

struct MapCell 
{
	// terrain flags
	unsigned char
		walkable : 1,
		shootable : 1,
		water : 1;

	// dynamic official flags
	unsigned char
		npc : 1,
		basilica : 1,
		landprotector : 1,
		maelstrom : 1;
	
	// custom flags
#ifdef ENABLE_CUSTOM_CELLFLAGS
	unsigned char
		novending : 1,
		nochat : 1,
#endif
};

// MapData
struct MapData 
{
	// Map name
	char name[MAP_NAME_LENGTH];

	// MapCell array
	struct MapCell* cell;

	// MapIndex ID, Width, Height(in cells)
	short m, w, h;

	// Number of players on the map
	int users;

	// MapFlags
	struct map_flag 
	{
		// PvP Flags
		unsigned pvp : 1;
		unsigned pvp_noparty : 1;
		unsigned pvp_noguild : 1;
		unsigned pvp_nightmaredrop :1;
		unsigned pvp_nocalcrank : 1;

		// GvG Flags
		unsigned gvg_castle : 1;
		unsigned gvg : 1;
		unsigned gvg_dungeon : 1;
		unsigned gvg_noparty : 1;
	} flags;
};

// eAthena MapCache file header
struct MapCacheHeader 
{
	unsigned int file_size;
	unsigned short map_count;
};

// eAthena MapCache Map Descriptor
struct MapCacheMapInfo 
{
	char name[MAP_NAME_LENGTH];
	unsigned short xs;
	unsigned short ys;
	unsigned int len;
};

class MapManager
{
public:
	static vector<MapData> maps;
	static map_index mapsidx;

	static void initialize();
	static bool decode_mapcache(struct MapData *m, char *buffer, char *decode_buffer);

	static int get_map_by_index(int m)
	{
		for (int i = 0; i < maps.size(); i++)
		{
			if (maps[i].m == m)
				return i;
		}

		return -1;
	}
};
