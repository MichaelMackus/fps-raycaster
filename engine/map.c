#include "map.h"

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

    // TODO free texture

    free(map);
}

const Tile* get_tile(const Map *map, int x, int y)
{
    if (x >= map->width)
    {
        printf("out of bounds\n");
        return NULL;
    }
    if (y >= map->height)
    {
        printf("out of bounds\n");
        return NULL;
    }

    return &(map->tiles[y*map->width + x]);
}

int is_passable(const Map *map, int x, int y)
{
    const Tile *t = get_tile(map, x, y);

    if (t == NULL)
        return 0;

    return t->type == TILE_PASSABLE;
}
