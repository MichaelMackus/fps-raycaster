#ifndef ENGINE_MAP_H
#define ENGINE_MAP_H

#include "stdinc.h"

// TODO get these from engine
#define MAP_WIDTH 19
#define MAP_HEIGHT 19
#define MAP_MAX_DISTANCE MAP_WIDTH*MAP_WIDTH + MAP_HEIGHT*MAP_HEIGHT

const char* get_map();
const char get_tile(int x, int y);

#endif
