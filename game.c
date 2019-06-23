#include "game.h"
#include "engine/map.h"
#include "engine/entity.h"

#include <stdio.h>

int init_game()
{
    Player *player = get_player();
    player->pos.x = 9.5;
    player->pos.y = 9.5;
    player->dir = to_radians(90);
    player->fov = to_radians(90);

    // read map from file
    {
        FILE *f = fopen("map.txt", "r");
        if (f == NULL)
        {
            return 1;
        }

        // curMap is our current map buffer (to write), line is the current file read buffer
        char *curMap = get_map();
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
