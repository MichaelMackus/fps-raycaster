#include "game.h"
#include "engine/draw.h"
#include "engine/map.h"
#include "engine/entity.h"
#include "engine/raycast.h"
#include "engine/stdinc.h"

#include <stdio.h>

int screenWidth;
int screenHeight;

// z-index of drawn walls
static double* wallZ;

// multiple different texture atlas for pre-rendered lighting of map textures
TextureAtlas *textureAtlas;
TextureAtlas *textureAtlas1;
TextureAtlas *textureAtlas2;
TextureAtlas *textureAtlas3;
// sprites textures
TextureAtlas *spritesAtlas;
TextureAtlas *projectilesAtlas;

// sort function for sorting enemies by depth
int sort_enemies(const Sprite *e1, const Sprite *e2)
{
    if (e1->distY > e2->distY)
        return -1;
    else if (e1->distY == e2->distY)
        return 0;
    else
        return 1;
}

int init_game()
{
    Player *player = get_player();
    player->pos.x = 9.5;
    player->pos.y = 9.5;
    player->dir = to_radians(90);
    player->fov = to_radians(90);
    player->shooting = 0;

    get_screen_dimensions(&screenWidth, &screenHeight);

    // initialize wallZ array
    wallZ = malloc(sizeof(*wallZ) * screenWidth);

    if (wallZ == NULL)
        return 1;


    // load textures
    {
        textureAtlas = create_atlas("wolftextures.png");
        textureAtlas1 = create_atlas("wolftextures.png");
        textureAtlas2 = create_atlas("wolftextures.png");
        textureAtlas3 = create_atlas("wolftextures.png");

        SDL_SetTextureColorMod(textureAtlas->texture, 125, 125, 125);
        SDL_SetTextureColorMod(textureAtlas1->texture, 170, 170, 170);
        SDL_SetTextureColorMod(textureAtlas2->texture, 210, 210, 210);

        if (textureAtlas == NULL)
            return 1;

        if (populate_atlas(textureAtlas, 64, 64) == 0)
            return 1;
        if (populate_atlas(textureAtlas1, 64, 64) == 0)
            return 1;
        if (populate_atlas(textureAtlas2, 64, 64) == 0)
            return 1;
        if (populate_atlas(textureAtlas3, 64, 64) == 0)
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

    // load projectiles
    {
        projectilesAtlas = create_atlas("fireball.jpg");

        if (projectilesAtlas == NULL)
            return 1;

        // convert to RGBA32 (so we can modify transparency)
        SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
        SDL_Surface *newSurface = SDL_ConvertSurface(projectilesAtlas->surface, format, 0);
        SDL_FreeFormat(format);
        projectilesAtlas->surface = newSurface;

        // make sprite outlines transparent
        for (int x = 0; x < projectilesAtlas->width; ++x)
        {
            for (int y = 0; y < projectilesAtlas->height; ++y)
            {
                Color *c = &projectilesAtlas->pixels[x + y*projectilesAtlas->surface->w];
                if (c->r > 150 && c->g > 150 && c->b > 150)
                {
                    c->a = 0;
                }
            }
        }

        if (update_colors(projectilesAtlas->pixels, projectilesAtlas->surface) != 0)
            return 1;

        SDL_Texture *spritesTexture = SDL_CreateTextureFromSurface(get_renderer(), projectilesAtlas->surface);
        SDL_SetTextureBlendMode(spritesTexture, SDL_BLENDMODE_BLEND);

        if (spritesTexture == NULL)
            return 1;

        SDL_DestroyTexture(projectilesAtlas->texture);
        projectilesAtlas->texture = spritesTexture;

        if (populate_atlas(projectilesAtlas, 140, 140) == 0)
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

        int j = 0;
        for (int i = 0; i < size; ++i)
        {
            // don't append to map if newline
            if (line[i] == '\n') continue;

            switch (line[i]) {
                // wall tile
                case '#':
                    curTile->type = TILE_SOLID;
                    curTile->texture = textureAtlas->subtextures[3];
                    break;

                // flag tile
                case '&':
                    curTile->type = TILE_SOLID;
                    curTile->texture = textureAtlas->subtextures[0];
                    break;

                // floor tile
                default:
                    curTile->type = TILE_PASSABLE;
                    curTile->texture = textureAtlas->subtextures[6];
            }

            curTile ++;
            bytesRead ++;
            j ++;
        }
    }

    // load enemies
    {
        Sprite *enemies = malloc(sizeof(*enemies) * SPRITE_COUNT);

        if (enemies == NULL)
            return map;

        for (int i = 0; i < SPRITE_COUNT; ++i) enemies[i].texture = NULL;

        enemies[0] = (Sprite) { spritesAtlas->subtextures[83], SPRITE_ENEMY, 0, 0, 0, { 3, 3 }, 0 };
        enemies[1] = (Sprite) { spritesAtlas->subtextures[15], SPRITE_ENEMY, 0, 0, 0, { 8, 16 }, 0 };
        enemies[2] = (Sprite) { spritesAtlas->subtextures[11], SPRITE_ENEMY, 0, 0, 0, { 6, 17 }, 0 };
        enemies[3] = (Sprite) { spritesAtlas->subtextures[15], SPRITE_ENEMY, 0, 0, 0, { 6, 6 }, 0 };
        enemies[4] = (Sprite) { spritesAtlas->subtextures[15], SPRITE_ENEMY, 0, 0, 0, { 9, 2 }, 0 };

        map->entities = enemies;
        map->entityCount = 5;
    }

    return map;
}

int do_raycast(Map *map)
{
    Player *player = get_player();

    // calculate distance from player to screen - this will be screenWidth/2 for 90 degree FOV
    double distanceToSurface = (screenWidth/2.0) / tan(player->fov/2);

    draw_init_layer(1, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1);
    draw_init_layer(2, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1);
    draw_init_layer(3, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1);

    // draw the floor and the walls
    {
        // detect which squares the player can see, and draw them proportionally to distance
        // get texture & lock for streaming
        SDL_Texture *streamTexture = get_texture(1);
        Uint32 *pixels;
        int pitch;
        SDL_LockTexture(streamTexture, NULL, (void**) &pixels, &pitch);
        memset(pixels, 0, pitch * screenHeight); // clear streaming target

        // draw floor scanlines
        for (int y = 0; y < screenHeight/2; y++)
        {
            Vector ray = floorcast(map, y);
            Vector interval = floorcast_get_interval(map, y);

            for (int x = 0; x < screenWidth; x++)
            {
                // get the floor tile
                const Tile *tile = get_tile(map, (int)(ray.x), (int)(ray.y));
                if (tile == NULL ||
                    tile->type != TILE_PASSABLE) // cheap trick to ensure we don't draw walls as floor
                {
                    ray.x += interval.x;
                    ray.y += interval.y;

                    continue;
                }

                // get colors from tile's subtexture
                Color *colors = tile->texture->pixels;

                // get the texture coordinate from the fractional part
                // TODO shouldn't need abs, but floorPos going negative
                int tx = (int)abs(tile->texture->width * (ray.x - (int)ray.x));
                int ty = (int)abs(tile->texture->height * (ray.y - (int)ray.y));

                ray.x += interval.x;
                ray.y += interval.y;

                // TODO will this work with all source pixel formats?
                const unsigned int offset = (pitch/sizeof(Uint32))*y + x;
                const unsigned int textureOffset = tile->texture->atlas->width * ty + tx;
                pixels[offset] = (0x3F << 24) |
                    (colors[textureOffset].r << 16) |
                    (colors[textureOffset].g << 8) |
                    (colors[textureOffset].b);

                const unsigned int floorOffset = (pitch/sizeof(Uint32))*(screenHeight - y -1) + x;
                pixels[floorOffset] = (0xCF << 24) |
                    (colors[textureOffset].r << 16) |
                    (colors[textureOffset].g << 8) |
                    (colors[textureOffset].b);
            }
        }

        SDL_UnlockTexture(streamTexture);

        // draw walls
        for (int x = 0; x < screenWidth; ++x)
        {
            Ray ray = raycast(map, x);
            Vector rayPos = ray.rayPos;

            double propDist = ray.distance;
            wallZ[x] = propDist;

            // calculate wall proportion percentage
            double proportion = 1 / propDist;
            if (proportion < 0)
                proportion = 0;

            // calculate wall height & ypos
            double wallHeight = screenHeight * proportion;
            double y = (screenHeight - wallHeight) / 2;

            double wallX = ray.xOffset;

            // get the texture from the map tile
            Vector tilePos = ray.tilePos;
            const Tile *t = get_tile(map, tilePos.x, tilePos.y);

            if (t == NULL)
            {
#ifdef GAME_DEBUG
                printf("Error - wall tile is null\n");
#endif
                return 0;
            }

            // offset from within texture (we're only rendering 1 slice of the wall)
            int textureX = wallX * t->texture->width;

            // draw walls on layer 2 (above floor/ceiling)
            draw_start(3);

            // add simple lighting to add definition for cube edges
            SDL_Texture *texture;
            if (ray.side == WALL_SOUTH || ray.side == WALL_NORTH)
                texture = textureAtlas->texture;
            else
                texture = textureAtlas2->texture;

            // draw texture
            draw_texture(texture,
                    t->texture->xOffset + textureX, t->texture->yOffset, 1, t->texture->height,
                    x, y, 1, wallHeight);
        }
    }
    // end draw the floor and the walls

    // draw enemies & objects
    {
        // if player has started shooting, spawn the projectile
        if (player->shooting && map->entityCount < SPRITE_COUNT)
        {
            map->entities[map->entityCount] =
                (Sprite) { projectilesAtlas->subtextures[0], SPRITE_PROJECTILE, 0, 0, 0, player->pos, player->dir };
            ++(map->entityCount);
        }
        player->shooting = 0;

        Sprite *enemies = map->entities;

        // setup enemy distance & angle for sorting
        for (int i = 0; i < map->entityCount; ++i)
        {
            Sprite *enemy = &enemies[i];
            Vector enemyPos = enemies[i].pos;
            // calculate angle to enemy using dot product
            Vector normPlayer = (Vector) { cos(player->dir), sin(player->dir) };
            Vector venemy = (Vector) { enemyPos.x - player->pos.x, enemyPos.y - player->pos.y };
            Vector normEnemy = normalize(venemy);

            // *cross* product to calculate which side of player
            double product = -1*normPlayer.y*normEnemy.x + normPlayer.x*normEnemy.y;
            if (product > 0)
                enemy->side = 1; // right side
            else
                enemy->side = -1; // right side

            // dist is euclidian distance (from player to enemy)
            enemy->angle = acos(dot_product(normPlayer, normEnemy));
            double dist = distance(player->pos, enemyPos);
            // distX & distY are perpendicular distance from camera in X & Y
            enemy->distX = sin(enemy->angle) * dist;
            enemy->distY = cos(enemy->angle) * dist;
        }

        // sort the enemies by distance
        qsort(&enemies[0], map->entityCount, sizeof(Sprite), (const void*) sort_enemies);

        draw_start(3); // layer 3 - sprites (renderer target)
        for (int i = 0; i < map->entityCount; ++i)
        {
            // if this is a projectile, move it forward & perform collision detection
            if (enemies[i].type == SPRITE_PROJECTILE)
            {
                // look for an enemy in this position
                for (int j = 0; j < map->entityCount; ++j)
                {
                    if (enemies[j].type == SPRITE_ENEMY &&
                        (int)(enemies[i].pos.x) == (int)(enemies[j].pos.x) &&
                        (int)(enemies[i].pos.x) == (int)(enemies[j].pos.x))
                    {
                        enemies[j].pos.x = -1;
                        enemies[j].pos.y = -1;
                    }
                }
                enemies[i].pos.x += cos(enemies[i].dir)/10.0;
                enemies[i].pos.y += sin(enemies[i].dir)/10.0;
            }

            Sprite enemy = enemies[i];
            Vector enemyPos = enemy.pos;

            // midX & midY are middle of the screen
            double midX = screenWidth / 2;
            double midY = screenHeight / 2;

            // calculate proportional (perpendicular) distance from player to enemy
            double proportion = 1 / enemy.distY;
            if (proportion < 0)
                proportion = 0;
            double enemyHeight = screenHeight * proportion;

            // https://www.reddit.com/r/gamedev/comments/4s7meq/rendering_sprites_in_a_raycaster/
            // Generally to project a 3D point to a 2D plane you do x2d = x3d *
            // projection_plane_distance / z3d (same for y2d and y3d). Almost
            // everything in a raycaster boils down to that
            //
            double spriteX = (midX - enemyHeight/2) + enemy.side * (enemy.distX*distanceToSurface/enemy.distY);
            double spriteY = midY - enemyHeight/2;

            if (enemy.angle <= player->fov)
            {
                // draw texture column by column, only if z value higher than wallZ
                int textureOffsetX = enemy.texture->xOffset;
                int textureOffsetY = enemy.texture->yOffset;
                double step = enemyHeight / enemy.texture->width;
                for (int x = 0; x < enemy.texture->width; ++x)
                {
                    // protect drawing past screen edges
                    int screenColumn = (int) (spriteX + x*step);
                    if (screenColumn < 0 || screenColumn >= screenWidth) continue;

                    // skip drawing over closer walls
                    if (wallZ[screenColumn] <= enemy.distY) continue;

                    // draw sprite
                    draw_texture(enemy.texture->atlas->texture,
                            textureOffsetX + x, textureOffsetY, 1, enemy.texture->height,
                            spriteX + x*step, spriteY, ceil(step), enemyHeight);
                }
            }
        }
    }
    // end draw enemies

    // finish drawing
    draw_update(3);

    return 1;
}
