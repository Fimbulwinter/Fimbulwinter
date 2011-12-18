#pragma once

#include <map>
#include "MapManager.hpp"

using namespace std;

class BlockManager
{
public:
	static map<int, struct BlockList *> id2bl;
	static map<int, struct ZoneSessionData *> id2pc;

	static void add_block(struct BlockList *bl)
	{
		if (bl == 0)
			return;

		id2bl[bl->id] = bl;
	}

	static void remove_block(struct BlockList *bl)
	{
		if (bl == 0)
			return;

		if (id2bl.count(bl->id))
			id2bl.erase(bl->id);
	}

	static struct BlockList *get_block(int id)
	{
		if (id2bl.count(id))
			return id2bl[id];

		return 0;
	}

	static ZoneSessionData *get_session(int account_id)
	{
		if (id2pc.count(account_id))
			return id2pc[account_id];

		return 0;
	}

private:

};
