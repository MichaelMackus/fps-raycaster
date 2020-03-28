#ifndef ENGINE_MAP_H
#define ENGINE_MAP_H

#include "texture.h"
#include "entity.h"
#include "stdinc.h"

typedef enum {
    TILE_PASSABLE,
    TILE_SOLID
} TileType;

typedef struct {
    SubTexture *texture;
    TileType type;
} Tile;

typedef struct {
    int width;
    int height;
    Tile *tiles;
    Entity *entities;
} Map;

// allocate memory for map & tiles
Map* create_map(int width, int height);

// free memory
void free_map(Map *map);

// return NULL if outside bounds
const Tile* get_tile(const Map *map, int x, int y);

// return 1 if tile at location is passable
int is_passable(const Map *map, int x, int y);

#endif
