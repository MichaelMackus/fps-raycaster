#include "assert.h"
#include "map.h"
#include "stdlib.h"

Map* create_map(int width, int height)
{
    Map *map = malloc(sizeof(*map));

    if (map != NULL)
    {
        map->width = width;
        map->height = height;
        map->tiles = malloc(sizeof(*(map->tiles)) * width * height);

        if (map->tiles == NULL)
        {
            free(map);
        }
    }

    return map;
}

void free_map(Map *map)
{
    if (map->tiles != NULL)
    {
        free(map->tiles);
    }

    free(map);
}

const Tile get_tile(const Map *map, int x, int y)
{
    assert(is_in_bounds(map, x, y));

    return map->tiles[y*map->width + x];
}

int is_passable(const Map *map, int x, int y)
{
    const Tile t = get_tile(map, x, y);

    return t == TILE_PASSABLE;
}

int is_in_bounds(const Map *map, int x, int y)
{
    return x < map->width && y < map->height && x >= 0 && y >= 0;
}
