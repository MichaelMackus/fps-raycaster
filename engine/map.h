#ifndef ENGINE_MAP_H
#define ENGINE_MAP_H

#include "stdinc.h"

typedef enum {
    TILE_PASSABLE,
    TILE_SOLID
} Tile;

typedef struct {
    int width;
    int height;
    Tile *tiles;
} Map;

// allocate memory for map & tiles
Map* create_map(int width, int height);

// free memory
void free_map(Map *map);

// error if outside bounds
const Tile get_tile(const Map *map, int x, int y);

// return 1 if tile at location is passable, error if outside bounds
int is_passable(const Map *map, int x, int y);

// return 1 if within map bounds
int is_in_bounds(const Map *map, int x, int y);

#endif
