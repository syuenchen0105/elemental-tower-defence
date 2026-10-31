#ifndef MAP_H
#define MAP_H

#include "models.h"

// Load the enemy path and tower placement spots from map.json
// Return true if the map is valid and loaded successfully
bool load_map(game_data &game);

#endif