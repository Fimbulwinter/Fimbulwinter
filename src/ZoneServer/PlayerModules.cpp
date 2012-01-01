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
*                    Player Manipulation Modules          	        *
* ==================================================================*/

#include "ZoneServer.hpp"
#include "PlayerModules.hpp"

bool PC::auth_ok(ZoneSessionData * sd, unsigned int login_id2, time_t expiration_time, int gmlevel, struct CharData * st)
{
	int i;

	sd->login_id2 = login_id2;
	sd->gmlevel = gmlevel;
	memcpy(&sd->status, st, sizeof(*st));

	if (st->sex != sd->status.sex) 
	{
		ZoneServer::auth_fail(sd->cl, 0);
		return false;
	}

	if ((i = set_pos(sd,sd->status.last_point.map, sd->status.last_point.x, sd->status.last_point.y, CLR_OUTSIGHT)) != 0) 
	{
		ShowError ("Last_point_map %s - id %d not found (error code %d)\n", MapManager::mapsidx.get_map_name(sd->status.last_point.map), sd->status.last_point.map, i);

		// try warping to a default map instead (church graveyard)
		if (set_pos(sd, MapManager::mapsidx.get_map_id(MAP_PRONTERA), 273, 354, CLR_OUTSIGHT) != 0) 
		{
			// if we fail again
			ZoneServer::auth_fail(sd->cl, 0);
			return false;
		}
	}

	ZoneServer::auth_ok(sd);

	ShowInfo("'"CL_WHITE"%s"CL_RESET"' logged in."
		" (AID/CID: '"CL_WHITE"%d/%d"CL_RESET"').\n",
		sd->status.name, sd->status.account_id, sd->status.char_id);

	ZoneServer::request_registry(sd, 7);

	return true;
}

void PC::auth_fail(ZoneSessionData * sd)
{
	ZoneServer::auth_fail(sd->cl, 0);
}

int PC::set_pos( ZoneSessionData * sd, unsigned short mapindex, short x, short y, ClearType clrtype )
{
	int m;

	if( !mapindex || !MapManager::mapsidx.get_map_name(mapindex) )
	{
		ShowDebug("PC::set_pos: Passed mapindex(%d) is invalid!\n", mapindex);
		return 1;
	}

	m = MapManager::get_map_by_index(mapindex);

	if (m < 0)
	{
		// Request to InterServer another MapServer
		return 2;
	}

	if( x < 0 || x >= MapManager::maps[m].w || y < 0 || y >= MapManager::maps[m].h )
	{
		ShowError("PC::set_pos: attempt to place player %s (%d:%d) on invalid coordinates (%s-%d,%d)\n", sd->status.name, sd->status.account_id, sd->status.char_id, MapManager::mapsidx.get_map_name(mapindex),x,y);
		x = y = 0; // make it random
	}

	if( x == 0 && y == 0 )
	{
		// pick a random walkable cell
		do 
		{
			x = rand() % (MapManager::maps[m].w - 2) + 1;
			y = rand() % (MapManager::maps[m].h - 2) + 1;
		} while(MapManager::check_cell(m,x,y,CELL_CHKNOPASS));
	}

	if (sd->bl.prev != NULL)
	{
		// Remove from actual map
		// Change map
	}

	sd->mapindex = mapindex;
	sd->bl.m = m;
	sd->bl.x = x;
	sd->bl.y = y; // TODO: Set unit data

	return 0;
}

void PC::reg_received( struct ZoneSessionData * sd )
{
	if (sd->state.active)
		return;

	sd->state.active = 1;

	BlockManager::add_block(&sd->bl);

	if (sd->state.connect_new == 0)
	{	
		//Character already loaded map! Gotta trigger LoadEndAck manually.
		sd->state.connect_new = 1;
		packet_loadendack(sd->cl, sd);
	}
}
