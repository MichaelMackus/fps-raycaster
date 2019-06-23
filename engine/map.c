#include "map.h"

static char map[MAP_HEIGHT*MAP_WIDTH];
const char* get_map()
{
    return map;
}
const char get_tile(int x, int y)
{
    return map[y*MAP_WIDTH + x];
}
