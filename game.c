#include "assert.h"
#include "game.h"
#include "engine/map.h"
#include "engine/entity.h"
#include "engine/raycast.h"
#include "engine/stdinc.h"

#include <stdio.h>

typedef struct WallSlize {
    int startY;
    int maxY;
    double wallHeight;
    double tstep;
    int textureX;
    double textureY;
    Ray ray;
    SubTexture *tileTex;
} WallSlice;

// rays to the drawn walls
static WallSlice* wallRays;

// multiple different texture atlas for pre-rendered shading of map textures
TextureAtlas *textureAtlas;
TextureAtlas *textureAtlas2;
// sprites textures
TextureAtlas *spritesAtlas;
TextureAtlas *projectilesAtlas;

// array of enemies
Entity *enemies;

// array of tiles for rendering
SubTexture **tileTextures;

// array of sprites for rendering
Sprite *sprites;

// sort function for sorting entities by depth
int sort_sprites(const Sprite *e1, const Sprite *e2)
{
    if (e1->dist > e2->dist)
        return -1;
    else if (e1->dist == e2->dist)
        return 0;
    else
        return 1;
}

SubTexture *get_tile_texture(Map *map, int x, int y)
{
    return tileTextures[y*map->width + x];
}

int init_game(int screenWidth, int screenHeight)
{
    Player *player = get_player();
    player->pos.x = 9.5;
    player->pos.y = 9.5;
    player->dir = to_radians(90);
    player->fov = to_radians(90);
    player->shooting = 0;

    sprites = malloc(sizeof(Sprite) * MAX_ENTITY_COUNT);

    // initialize wallRays array
    wallRays = malloc(sizeof(*wallRays) * screenWidth);

    if (wallRays == NULL)
        return 1;


    // load textures
    {
        textureAtlas = create_atlas("wolftextures.png");
        textureAtlas2 = create_atlas("wolftextures.png");

        /* SDL_SetTextureColorMod(textureAtlas->texture, 125, 125, 125); */ //TODO ??
        /* SDL_SetTextureColorMod(textureAtlas2->texture, 210, 210, 210); */

        if (textureAtlas == NULL)
            return 1;

        if (populate_atlas(textureAtlas, 64, 64) == 0)
            return 1;
        if (populate_atlas(textureAtlas2, 64, 64) == 0)
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
    tileTextures = malloc(sizeof(*tileTextures) * MAP_WIDTH * MAP_HEIGHT);
    SubTexture **curTileTexture = tileTextures;
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
                    *curTile = TILE_SOLID;
                    *curTileTexture = textureAtlas->subtextures[3];
                    break;

                // flag tile
                case '&':
                    *curTile = TILE_SOLID;
                    *curTileTexture = textureAtlas->subtextures[0];
                    break;

                // floor tile
                default:
                    *curTile = TILE_PASSABLE;
                    *curTileTexture = textureAtlas->subtextures[6];
            }

            curTile ++;
            curTileTexture ++;
            bytesRead ++;
            j ++;
        }
    }

    assert(tileTextures[18*MAP_WIDTH + 18] != NULL);

    // load enemies
    {
        enemies = malloc(sizeof(*enemies) * MAX_ENTITY_COUNT);

        if (enemies == NULL)
            return map;

        for (int i = 0; i < MAX_ENTITY_COUNT; ++i)
        {
            enemies[i].texture = NULL;
            enemies[i].health = 0;
            enemies[i].pos.x = -1;
            enemies[i].pos.y = -1;
        }

        enemies[0] = (Entity) { spritesAtlas->subtextures[83], ENTITY_ENEMY, { 3, 3 }, 0, 100 };
        enemies[1] = (Entity) { spritesAtlas->subtextures[15], ENTITY_ENEMY, { 8, 16 }, 0, 100 };
        enemies[2] = (Entity) { spritesAtlas->subtextures[11], ENTITY_ENEMY, { 6, 17 }, 0, 100 };
        enemies[3] = (Entity) { spritesAtlas->subtextures[15], ENTITY_ENEMY, { 6, 6 }, 0, 100 };
        enemies[4] = (Entity) { spritesAtlas->subtextures[15], ENTITY_ENEMY, { 9, 2 }, 0, 100 };
    }

    return map;
}

int do_raycast(Map *map, int screenWidth, int screenHeight, char *buffer)
{
    Player *player = get_player();

    // calculate distance from player to screen - this will be screenWidth/2 for 90 degree FOV
    double distanceToSurface = (screenWidth/2.0) / tan(player->fov/2);

    // draw the floor and the walls
    {
        // calculate rays to the walls
        for (int x = 0; x < screenWidth; ++x)
        {
            Ray ray = raycast(map, x);
            Vector rayPos = ray.rayPos;

            // calculate wall proportion percentage
            double propDist = ray.distance;
            if (propDist < 0)
                propDist = -1 * propDist;
            double proportion = 1 / propDist;

            // populate wall slice for rendering
            WallSlice wall;
            wall.wallHeight = screenHeight * proportion;
            double y = (screenHeight - wall.wallHeight) / 2;
            wall.maxY = (int)y + (int)wall.wallHeight;
            wall.startY = (int) y;
            wall.ray = ray;
            Vector tilePos = wall.ray.tilePos;
            assert(is_in_bounds(map, (int) tilePos.x, (int) tilePos.y));
            wall.tileTex = get_tile_texture(map, (int)(tilePos.x), (int)(tilePos.y));
            assert(wall.tileTex != NULL);
            assert(wall.tileTex->pixels != NULL);
            // offset from within texture (we're only rendering 1 wall of the wall)
            wall.tstep = (double)wall.tileTex->height / (double)wall.wallHeight;
            wall.textureX = ray.xOffset * (double)wall.tileTex->width;
            wall.textureY = 0;
            if (wall.startY < 0) {
                wall.textureY = (wall.tstep * (double)wall.startY * -1.0f);
            }

            wallRays[x] = wall;
        }

        // drawing code & floor/ceiling raycasting
        for (int y = 0; y < screenHeight; y++)
        {
            // TODO *floor* not in bounds of map
            FloorRay ray = floorcast(map, y);

            unsigned int offset = screenWidth*y*4;
            for (int x = 0; x < screenWidth; x++)
            {
                WallSlice wall = wallRays[x];

                // ensure ceiling is in map bounds & not drawing a wall
                if (y < wall.startY && is_in_bounds(map, (int) ray.tilePos.x, (int) ray.tilePos.y)) {
                    // TODO improve floorcasting performance
                    // get colors from tile's subtexture
                    SubTexture *tileTex = get_tile_texture(map, (int)(ray.tilePos.x), (int)(ray.tilePos.y));
                    assert(tileTex != NULL);
                    Color *colors = tileTex->pixels;
                    assert(colors != NULL);

                    // get the texture coordinate from the fractional part
                    // TODO shouldn't need abs, but floorPos going negative
                    int tx = (int)abs(tileTex->width * (ray.tilePos.x - (int)ray.tilePos.x));
                    int ty = (int)abs(tileTex->height * (ray.tilePos.y - (int)ray.tilePos.y));

                    const unsigned int textureOffset = tileTex->atlas->width * ty + tx;
                    buffer[offset + 0] = colors[textureOffset].r;
                    buffer[offset + 1] = colors[textureOffset].g;
                    buffer[offset + 2] = colors[textureOffset].b;
                    buffer[offset + 3] = 0x5F; // alpha

                    const unsigned int floorOffset = (screenHeight - y - 1) * screenWidth*4 + x*4;
                    buffer[floorOffset + 0] = colors[textureOffset].r;
                    buffer[floorOffset + 1] = colors[textureOffset].g;
                    buffer[floorOffset + 2] = colors[textureOffset].b;
                    buffer[floorOffset + 3] = 0xAF; // alpha
                }

                ray.tilePos.x += ray.xOffset;
                ray.tilePos.y += ray.yOffset;

                {
                    // get the texture from the map tile

                    /* double tstep = (double) tileTex->height / (double) wallHeight; // texture step from y-offset for each vertical screen pixel */
                    if (y >= wall.startY && y < wall.maxY) {
                        assert(wall.tileTex != NULL);
                        Color *colors = wall.tileTex->pixels;
                        assert(colors != NULL);

                        int textureOffset = (int)wall.textureX + (int)wall.textureY*wall.tileTex->atlas->width;
                        buffer[offset + 0] = colors[textureOffset].r;
                        buffer[offset + 1] = colors[textureOffset].g;
                        buffer[offset + 2] = colors[textureOffset].b;
                        // add simple lighting to add definition for cube edges
                        if (wall.ray.side == WALL_SOUTH || wall.ray.side == WALL_NORTH)
                            buffer[offset + 3] = colors[textureOffset].a; // alpha
                        else
                            buffer[offset + 3] = colors[textureOffset].a * 0.7; // alpha
                        wallRays[x].textureY += wall.tstep;
                    }
                }

                offset += 4;
            }
        }
    }
    // end draw the floor and the walls

    // draw enemies & objects
    // also moves projectiles and handles simplistic enemy AI
    /* { */
    /*     // if player has started shooting, spawn the projectile */
    /*     if (player->shooting > 0) */
    /*     { */
    /*         // find a spot for the new projectile entity */
    /*         for (int i = 0; i < MAX_ENTITY_COUNT; ++i) */
    /*         { */
    /*             if (enemies[i].health <= 0) */
    /*             { */
    /*                 enemies[i] = */
    /*                     (Entity) { projectilesAtlas->subtextures[0], ENTITY_PROJECTILE, player->pos, player->dir, 100 }; */
    /*                 break; */
    /*             } */
    /*         } */
    /*         // stop shooting (with a delay of 10 frames) */
    /*         player->shooting = -10; */
    /*     } */
    /*     // still recovering from recent shot */
    /*     else if (player->shooting < 0) */
    /*         ++player->shooting; */

    /*     // setup enemy distance & angle for sorting */
    /*     int spriteCount = 0; */
    /*     for (int i = 0; i < MAX_ENTITY_COUNT; ++i) */
    /*     { */
    /*         if (enemies[i].health <= 0) */
    /*             // this entity is dead (or does not exist) */
    /*             continue; */

    /*         // if this is a projectile, move it forward & perform collision detection */
    /*         if (enemies[i].type == ENTITY_PROJECTILE) */
    /*         { */
    /*             // look for an enemy in this position */
    /*             for (int j = 0; j < MAX_ENTITY_COUNT; ++j) */
    /*             { */
    /*                 if (enemies[j].type == ENTITY_ENEMY && */
    /*                     enemies[j].health > 0 && */
    /*                     check_collision(enemies[i].pos, enemies[j].pos, 1, 1)) */
    /*                 { */
    /*                     // collision found - hurt enemy & destroy projectile */
    /*                     enemies[j].health -= 50; */
    /*                     enemies[i].health = 0; */
    /*                 } */
    /*             } */
    /*             // move projectile */
    /*             if (enemies[i].pos.x > 0 && enemies[i].pos.y > 0 && */
    /*                 enemies[i].pos.x < map->width && enemies[i].pos.y < map->height) */
    /*             { */
    /*                 enemies[i].pos.x += cos(enemies[i].dir); */
    /*                 enemies[i].pos.y += sin(enemies[i].dir); */
    /*             } */
    /*             else */
    /*             { */
    /*                 // out of bounds, destroy projectile */
    /*                 enemies[i].health = 0; */
    /*                 continue; */
    /*             } */
    /*         } */

    /*         Entity entity = enemies[i]; */
    /*         Sprite sprite; */

    /*         // calculate angle to enemy using dot product */
    /*         Vector normPlayer = (Vector) { cos(player->dir), sin(player->dir) }; */
    /*         Vector venemy = (Vector) { entity.pos.x - player->pos.x, entity.pos.y - player->pos.y }; */
    /*         Vector normEnemy = normalize(venemy); */

    /*         // *cross* product to calculate which side of player */
    /*         double product = -1*normPlayer.y*normEnemy.x + normPlayer.x*normEnemy.y; */
    /*         if (product > 0) */
    /*             sprite.side = 1; // right side */
    /*         else */
    /*             sprite.side = -1; // right side */

    /*         // angle of enemy from player direction */
    /*         double angle = acos(dot_product(normPlayer, normEnemy)); */

    /*         // don't add sprite to screen if player cannot see it */
    /*         if (angle > player->fov) */
    /*             continue; */

    /*         // dist is euclidian distance (from player to enemy) */
    /*         // distX & distY are perpendicular distance from camera in X & Y */
    /*         double dist = distance(player->pos, entity.pos); */
    /*         double distX = sin(angle) * dist; */
    /*         double distY = cos(angle) * dist; */
    /*         sprite.dist = distY; */
    /*         sprite.entity = &(enemies[i]); */

    /*         // build sprite struct and add to sprites array to render */
    /*         double proportion = 1 / distY; */
    /*         if (proportion < 0) proportion = 0; */
    /*         double spriteHeight = screenHeight * proportion; */
    /*         double midX = screenWidth / 2; */
    /*         double midY = screenHeight / 2; */
    /*         double spriteX = (midX - spriteHeight/2) + sprite.side * (distX*distanceToSurface/distY); */
    /*         double spriteY = midY - spriteHeight/2; */
    /*         sprite.screenPos.x = (int)spriteX; */
    /*         sprite.screenPos.y = (int)spriteY; */
    /*         sprite.height = (int)spriteHeight; */
    /*         sprites[spriteCount++] = sprite; */

    /*         // update enemy if can see the player (simple enemy AI) */
    /*         double angleToPlayer = rotate(player->dir, PI); */
    /*         if (sprite.side == 1) angleToPlayer = rotate(angleToPlayer, angle); */
    /*         else angleToPlayer = rotate(angleToPlayer, PI*2-angle); */
    /*         double newX = enemies[i].pos.x + cos(angleToPlayer)/20.0; */
    /*         double newY = enemies[i].pos.y + sin(angleToPlayer)/20.0; */
    /*         if (is_in_bounds(map, (int)newX, (int)newY) && is_passable(map, (int)newX, (int)newY) && sprite.dist > 1) */
    /*         { */
    /*             enemies[i].pos.x = newX; */
    /*             enemies[i].pos.y = newY; */
    /*         } */
    /*     } */

    /*     // sort the sprites by distance */
    /*     qsort(&sprites[0], spriteCount, sizeof(Sprite), (const void*) sort_sprites); */

    /*     draw_start(3); // layer 3 - sprites (renderer target) */
    /*     for (int i = 0; i < spriteCount; ++i) */
    /*     { */
    /*         Sprite sprite = sprites[i]; */

    /*         // draw texture column by column, only if z value higher than wallZ */
    /*         int textureOffsetX = sprite.entity->texture->xOffset; */
    /*         int textureOffsetY = sprite.entity->texture->yOffset; */
    /*         double step = (double)sprite.height / sprite.entity->texture->width; */
    /*         for (int x = 0; x < sprite.entity->texture->width; ++x) */
    /*         { */
    /*             // protect drawing past screen edges */
    /*             int screenColumn = (int) (sprite.screenPos.x + x*step); */
    /*             if (screenColumn < 0 || screenColumn >= screenWidth) continue; */

    /*             // skip drawing over closer walls */
    /*             if (wallZ[screenColumn] <= sprite.dist) continue; */

    /*             // draw sprite */
    /*             draw_texture(sprite.entity->texture->atlas->texture, */
    /*                     textureOffsetX + x, textureOffsetY, 1, sprite.entity->texture->height, */
    /*                     sprite.screenPos.x + x*step, sprite.screenPos.y, ceil(step), sprite.height); */
    /*         } */
    /*     } */
    /* } */
    /* // end draw enemies */

    /* // finish drawing */
    /* draw_update(3); */

    return 1;
}
