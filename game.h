#ifndef GAME_H
#define GAME_H

#define MAP_WIDTH 19
#define MAP_HEIGHT 19
#define ENEMY_COUNT 5

#include "engine/map.h"

// initialize game data
int init_game();

// initialize game map
Map* load_map(const char *fileName);

// do raycasting & draw
int do_raycast(const Map *map);

#endif
