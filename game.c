#include "game.h"
#include "engine/draw.h"
#include "engine/map.h"
#include "engine/entity.h"

#include <stdio.h>

SDL_Texture *textureAtlas;

int init_game()
{
    Player *player = get_player();
    player->pos.x = 9.5;
    player->pos.y = 9.5;
    player->dir = to_radians(90);
    player->fov = to_radians(90);

    // load textures
    {
        // load our texture image
        SDL_Surface *textureSurface = IMG_Load("wolftextures.png");
        
        if (textureSurface == NULL)
            return 1;

        // ensure format is RGBA with 32-bits for color manipulation
        SDL_Surface *tmp = SDL_ConvertSurfaceFormat(textureSurface, SDL_PIXELFORMAT_ARGB8888, 0);
        if (tmp == NULL) return 1;
        SDL_FreeSurface(textureSurface);
        textureSurface = tmp;

        textureAtlas = SDL_CreateTextureFromSurface(get_renderer(), textureSurface);

        if (textureAtlas == NULL)
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

            curTile->texture.atlas = textureAtlas;

            switch (line[i]) {
                // wall tile
                case '#':
                    curTile->type = TILE_SOLID;
                    curTile->texture.width = 64;
                    curTile->texture.height = 64;
                    curTile->texture.xOffset = 64;
                    curTile->texture.yOffset = 0;
                    break;

                // flag tile
                case '&':
                    curTile->type = TILE_SOLID;
                    curTile->texture.width = 64;
                    curTile->texture.height = 64;
                    curTile->texture.xOffset = 0;
                    curTile->texture.yOffset = 0;
                    break;

                // floor tile
                default:
                    curTile->type = TILE_PASSABLE;
                    curTile->texture.width = 64;
                    curTile->texture.height = 64;
                    curTile->texture.xOffset = 64*3;
                    curTile->texture.yOffset = 0;
            }

            curTile ++;
            bytesRead ++;
        }
    }

    return map;
}
