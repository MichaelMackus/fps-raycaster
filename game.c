#include "game.h"
#include "engine/draw.h"
#include "engine/map.h"
#include "engine/entity.h"

#include <stdio.h>

TextureAtlas *textureAtlas;
TextureAtlas *spritesAtlas;

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

    // load sprites
    {
        spritesAtlas = create_atlas("enemies.png");

        if (spritesAtlas == NULL)
            return 1;

        // make sprite outlines transparent
        for (int x = 0; x < spritesAtlas->width; ++x)
        {
            for (int y = 0; y < spritesAtlas->height; ++y)
            {
                Color *c = &spritesAtlas->pixels[x + y*spritesAtlas->surface->w];
                if (c->r > 125 && c->g > 125 && c->b > 125)
                {
                    c->a = 0;
                }
            }
        }

        if (update_colors(spritesAtlas->pixels, spritesAtlas->surface) != 0)
            return 1;

        SDL_Texture *spritesTexture = SDL_CreateTextureFromSurface(get_renderer(), spritesAtlas->surface);
        SDL_SetTextureBlendMode(spritesTexture, SDL_BLENDMODE_BLEND);

        if (spritesTexture == NULL)
            return 1;

        SDL_DestroyTexture(spritesAtlas->texture);
        spritesAtlas->texture = spritesTexture;

        if (populate_atlas(spritesAtlas, 61, 61) == 0)
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

    // load enemies
    {
        Sprite *enemies = malloc(sizeof(*enemies) * ENEMY_COUNT);

        if (enemies == NULL)
            return map;

        enemies[0] = (Sprite) { spritesAtlas->subtextures[83], 0, 0, 0, { 3, 3 } };
        enemies[1] = (Sprite) { spritesAtlas->subtextures[15], 0, 0, 0, { 8, 16 } };
        enemies[2] = (Sprite) { spritesAtlas->subtextures[11], 0, 0, 0, { 6, 17 } };
        enemies[3] = (Sprite) { spritesAtlas->subtextures[15], 0, 0, 0, { 6, 6 } };
        enemies[4] = (Sprite) { spritesAtlas->subtextures[15], 0, 0, 0, { 9, 2 } };
        map->entities = enemies;
        map->entityCount = ENEMY_COUNT;
    }

    return map;
}
