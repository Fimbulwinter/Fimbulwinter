#include "BlockManager.hpp"
#include "MapManager.hpp"
#include "ZoneServer.hpp"

map<int, struct BlockList *> BlockManager::id2bl;
map<int, struct ZoneSessionData *> BlockManager::id2pc;
