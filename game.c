#include "game.h"
#include "engine/draw.h"
#include "engine/map.h"
#include "engine/entity.h"

#include <stdio.h>

TextureAtlas *textureAtlas;

int init_game()
{
    Player *player = get_player();
    player->pos.x = 9.5;
    player->pos.y = 9.5;
    player->dir = to_radians(90);
    player->fov = to_radians(90);

    // load textures
    {
        textureAtlas = create_atlas("wolftextures.png");

        if (textureAtlas == NULL)
            return 1;

        if (populate_atlas(textureAtlas, 64, 64) == 0)
            return 1;
    }

    return 0;
}

Map* load_map(const char *fileName)
{
    // read map from file
    FILE *f = fopen(fileName, "r");
    if (f == NULL)
    {
        return NULL;
    }

    Map *map = create_map(MAP_WIDTH, MAP_HEIGHT);
    Tile *curTile = map->tiles;
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
            return NULL;

        for (int i = 0; i < size; ++i)
        {
            // don't append to map if newline
            if (line[i] == '\n') continue;

            switch (line[i]) {
                // wall tile
                case '#':
                    curTile->type = TILE_SOLID;
                    curTile->texture = textureAtlas->subtextures[1];
                    break;

                // flag tile
                case '&':
                    curTile->type = TILE_SOLID;
                    curTile->texture = textureAtlas->subtextures[0];
                    break;

                // floor tile
                default:
                    curTile->type = TILE_PASSABLE;
                    curTile->texture = textureAtlas->subtextures[3];
            }

            curTile ++;
            bytesRead ++;
        }
    }

    return map;
}
