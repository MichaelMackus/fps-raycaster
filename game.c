#include "draw.h"
#include "game.h"

#include <stdio.h>
#include "SDL.h"

static Player player;
Player* get_player()
{
    return &player;
}

static char map[MAP_HEIGHT*MAP_WIDTH];
const char* get_map()
{
    return map;
}
const char get_tile(int x, int y)
{
    return map[y*MAP_WIDTH + x];
}

// sprites
Sprite enemies[ENEMY_COUNT] = { 
    { 6*13 + 5, 0, 0, 0, { 3, 3 } },
    { 13 + 2, 0, 0, 0, { 8, 16 } },
    { 11, 0, 0, 0, { 6, 17 } },
    { 13 + 2, 0, 0, 0, { 6, 6 } },
    { 13 + 2, 0, 0, 0, { 9, 2 } }
};
Sprite* get_enemies()
{
    return enemies;
}

int init_game()
{
    player.pos.x = 9.5;
    player.pos.y = 9.5;
    player.dir = to_radians(90);
    player.fov = to_radians(90);

    // read map from file
    FILE *f = fopen("map.txt", "r");
    if (f == NULL) return 1;
    char *tmp = map; // current map buffer
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        // read MAP_WIDTH characters into map buffer
        if (fread(tmp, sizeof(*tmp), MAP_WIDTH, f) == 0) {
            return 1;
        }
        // skip newline
        fseek(f, 1, SEEK_CUR);
        // advance map buffer MAP_WIDTH characters
        tmp += MAP_WIDTH;
    }

    return 0;
}
