#include "assert.h"
#include "game.h"
#include "engine/draw.h"
#include "engine/map.h"
#include "engine/entity.h"
#include "engine/raycast.h"
#include "engine/stdinc.h"

#include <stdio.h>

// z-index of drawn walls
static double* wallZ;

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

int init_game()
{
    Player *player = get_player();
    player->pos.x = 9.5;
    player->pos.y = 9.5;
    player->dir = to_radians(90);
    player->fov = to_radians(90);
    player->shooting = 0;

    sprites = malloc(sizeof(Sprite) * MAX_ENTITY_COUNT);

    int screenWidth;
    int screenHeight;
    get_screen_dimensions(&screenWidth, &screenHeight);

    // initialize wallZ array
    wallZ = malloc(sizeof(*wallZ) * screenWidth);

    if (wallZ == NULL)
        return 1;


    // load textures
    {
        textureAtlas = create_atlas("wolftextures.png");
        textureAtlas2 = create_atlas("wolftextures.png");

        SDL_SetTextureColorMod(textureAtlas->texture, 125, 125, 125);
        SDL_SetTextureColorMod(textureAtlas2->texture, 210, 210, 210);

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

int do_raycast(Map *map)
{
    Player *player = get_player();

    int screenWidth;
    int screenHeight;
    get_screen_dimensions(&screenWidth, &screenHeight);

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
                SubTexture *tileTex = get_tile_texture(map, (int)(ray.x), (int)(ray.y));

                // cheap trick to ensure we don't draw walls as floor
                if (!is_in_bounds(map, (int) ray.x, (int) ray.y) || !is_passable(map, (int) ray.x, (int) ray.y) || tileTex == NULL)
                {
                    ray.x += interval.x;
                    ray.y += interval.y;

                    continue;
                }

                // get colors from tile's subtexture
                Color *colors = tileTex->pixels;
                assert(colors != NULL);

                // get the texture coordinate from the fractional part
                // TODO shouldn't need abs, but floorPos going negative
                int tx = (int)abs(tileTex->width * (ray.x - (int)ray.x));
                int ty = (int)abs(tileTex->height * (ray.y - (int)ray.y));

                ray.x += interval.x;
                ray.y += interval.y;

                // TODO will this work with all source pixel formats?
                const unsigned int offset = (pitch/sizeof(Uint32))*y + x;
                const unsigned int textureOffset = tileTex->atlas->width * ty + tx;
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
            assert(is_in_bounds(map, (int) tilePos.x, (int) tilePos.y));
            SubTexture *tileTex = get_tile_texture(map, (int)(tilePos.x), (int)(tilePos.y));
            assert(tileTex != NULL);
            Color *colors = tileTex->pixels;
            assert(colors != NULL);

            // offset from within texture (we're only rendering 1 slice of the wall)
            int textureX = wallX * tileTex->width;

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
                    tileTex->xOffset + textureX, tileTex->yOffset, 1, tileTex->height,
                    x, y, 1, wallHeight);
        }
    }
    // end draw the floor and the walls

    // draw enemies & objects
    // also moves projectiles and handles simplistic enemy AI
    {
        // if player has started shooting, spawn the projectile
        if (player->shooting > 0)
        {
            // find a spot for the new projectile entity
            for (int i = 0; i < MAX_ENTITY_COUNT; ++i)
            {
                if (enemies[i].health <= 0)
                {
                    enemies[i] =
                        (Entity) { projectilesAtlas->subtextures[0], ENTITY_PROJECTILE, player->pos, player->dir, 100 };
                    break;
                }
            }
            // stop shooting (with a delay of 10 frames)
            player->shooting = -10;
        }
        // still recovering from recent shot
        else if (player->shooting < 0)
            ++player->shooting;

        // setup enemy distance & angle for sorting
        int spriteCount = 0;
        for (int i = 0; i < MAX_ENTITY_COUNT; ++i)
        {
            if (enemies[i].health <= 0)
                // this entity is dead (or does not exist)
                continue;

            // if this is a projectile, move it forward & perform collision detection
            if (enemies[i].type == ENTITY_PROJECTILE)
            {
                // look for an enemy in this position
                for (int j = 0; j < MAX_ENTITY_COUNT; ++j)
                {
                    if (enemies[j].type == ENTITY_ENEMY &&
                        enemies[j].health > 0 &&
                        check_collision(enemies[i].pos, enemies[j].pos, 1, 1))
                    {
                        // collision found - hurt enemy & destroy projectile
                        enemies[j].health -= 50;
                        enemies[i].health = 0;
                    }
                }
                // move projectile
                if (enemies[i].pos.x > 0 && enemies[i].pos.y > 0 &&
                    enemies[i].pos.x < map->width && enemies[i].pos.y < map->height)
                {
                    enemies[i].pos.x += cos(enemies[i].dir);
                    enemies[i].pos.y += sin(enemies[i].dir);
                }
                else
                {
                    // out of bounds, destroy projectile
                    enemies[i].health = 0;
                    continue;
                }
            }

            Entity entity = enemies[i];
            Sprite sprite;

            // calculate angle to enemy using dot product
            Vector normPlayer = (Vector) { cos(player->dir), sin(player->dir) };
            Vector venemy = (Vector) { entity.pos.x - player->pos.x, entity.pos.y - player->pos.y };
            Vector normEnemy = normalize(venemy);

            // *cross* product to calculate which side of player
            double product = -1*normPlayer.y*normEnemy.x + normPlayer.x*normEnemy.y;
            if (product > 0)
                sprite.side = 1; // right side
            else
                sprite.side = -1; // right side

            // angle of enemy from player direction
            double angle = acos(dot_product(normPlayer, normEnemy));

            // don't add sprite to screen if player cannot see it
            if (angle > player->fov)
                continue;

            // dist is euclidian distance (from player to enemy)
            // distX & distY are perpendicular distance from camera in X & Y
            double dist = distance(player->pos, entity.pos);
            double distX = sin(angle) * dist;
            double distY = cos(angle) * dist;
            sprite.dist = distY;
            sprite.entity = &(enemies[i]);

            // build sprite struct and add to sprites array to render
            double proportion = 1 / distY;
            if (proportion < 0) proportion = 0;
            double spriteHeight = screenHeight * proportion;
            double midX = screenWidth / 2;
            double midY = screenHeight / 2;
            double spriteX = (midX - spriteHeight/2) + sprite.side * (distX*distanceToSurface/distY);
            double spriteY = midY - spriteHeight/2;
            sprite.screenPos.x = (int)spriteX;
            sprite.screenPos.y = (int)spriteY;
            sprite.height = (int)spriteHeight;
            sprites[spriteCount++] = sprite;

            // update enemy if can see the player (simple enemy AI)
            double angleToPlayer = rotate(player->dir, PI);
            if (sprite.side == 1) angleToPlayer = rotate(angleToPlayer, angle);
            else angleToPlayer = rotate(angleToPlayer, PI*2-angle);
            double newX = enemies[i].pos.x + cos(angleToPlayer)/20.0;
            double newY = enemies[i].pos.y + sin(angleToPlayer)/20.0;
            if (is_in_bounds(map, (int)newX, (int)newY) && is_passable(map, (int)newX, (int)newY) && sprite.dist > 1)
            {
                enemies[i].pos.x = newX;
                enemies[i].pos.y = newY;
            }
        }

        // sort the sprites by distance
        qsort(&sprites[0], spriteCount, sizeof(Sprite), (const void*) sort_sprites);

        draw_start(3); // layer 3 - sprites (renderer target)
        for (int i = 0; i < spriteCount; ++i)
        {
            Sprite sprite = sprites[i];

            // draw texture column by column, only if z value higher than wallZ
            int textureOffsetX = sprite.entity->texture->xOffset;
            int textureOffsetY = sprite.entity->texture->yOffset;
            double step = (double)sprite.height / sprite.entity->texture->width;
            for (int x = 0; x < sprite.entity->texture->width; ++x)
            {
                // protect drawing past screen edges
                int screenColumn = (int) (sprite.screenPos.x + x*step);
                if (screenColumn < 0 || screenColumn >= screenWidth) continue;

                // skip drawing over closer walls
                if (wallZ[screenColumn] <= sprite.dist) continue;

                // draw sprite
                draw_texture(sprite.entity->texture->atlas->texture,
                        textureOffsetX + x, textureOffsetY, 1, sprite.entity->texture->height,
                        sprite.screenPos.x + x*step, sprite.screenPos.y, ceil(step), sprite.height);
            }
        }
    }
    // end draw enemies

    // finish drawing
    draw_update(3);

    return 1;
}
