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
    {
        FILE *f = fopen("map.txt", "r");
        if (f == NULL)
        {
            return 1;
        }

        // curMap is our current map buffer (to write), line is the current file read buffer
        char *curMap = map;
        char line[MAP_WIDTH];

        for (int bytesRead = 0; bytesRead < MAP_WIDTH*MAP_HEIGHT;)
        {
            // read bytesToRead characters into map buffer
            int bytesToRead = MAP_WIDTH;
            if (bytesRead + bytesToRead > MAP_WIDTH*MAP_HEIGHT)
            {
                bytesToRead = MAP_WIDTH*MAP_HEIGHT - bytesRead;
            }
            int size = fread(line, sizeof(*line), bytesToRead, f);

            // EOF before end of map
            if (size == 0)
                return 1;

            for (int i = 0; i < size; ++i)
            {
                // don't append to map if newline
                if (line[i] == '\n') continue;

                *curMap = line[i];
                curMap ++;
                bytesRead ++;
            }
        }
    }

    return 0;
}
