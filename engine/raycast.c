#include "draw.h"
#include "raycast.h"
#include "entity.h"
#include "map.h"
#include "stdinc.h"

#include "SDL.h"

int screenWidth;
int screenHeight;

// z-index of drawn walls
static double* wallZ;

int init_raycast()
{
    get_screen_dimensions(&screenWidth, &screenHeight);

    // initialize wallZ array
    wallZ = malloc(sizeof(*wallZ) * screenWidth);

    if (wallZ == NULL)
        return 1;

    return 0;
}

int destroy_raycast()
{
    free(wallZ);

    return 0;
}

typedef enum {
    WALL_NORTH,
    WALL_EAST,
    WALL_WEST,
    WALL_SOUTH
} WallSide;

// whether vector hits wall side
int hit_wall(const Map *map, Vector pos, WallSide side)
{
    if (side == WALL_SOUTH)
    {
        pos.y -= 1;
    }

    if (side == WALL_EAST)
    {
        pos.x -= 1;
    }

    return is_passable(map, pos.x, pos.y) == 0;
}

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

int raycast(const Map *map)
{
    Player *player = get_player();

    // calculate distance from player to screen - this will be screenWidth/2 for 90 degree FOV
    double distanceToSurface = (screenWidth/2.0) / tan(player->fov/2);

    draw_init_layer(1, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1);
    draw_init_layer(2, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1);
    draw_init_layer(3, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1);

    // draw the floor and the walls
    {
        // detect which squares the player can see, and draw them proportionally to distance
        // get texture & lock for streaming
        SDL_Texture *streamTexture = get_texture(2);
        Uint32 *pixels;
        int pitch;
        SDL_LockTexture(streamTexture, NULL, (void**) &pixels, &pitch);
        memset(pixels, 0, pitch * screenHeight); // clear streaming target

        // detect which squares the player can see, and draw them proportionally to distance
        for (int x = 0; x < screenWidth; ++x)
        {
            // calculate ray direction
            /* double rayDir = player->dir - (player->fov/2) + x * (player->fov/screenWidth); // generates distortion towards edges */
            // fix for increased distortion towards screen edges
            // see: https://stackoverflow.com/questions/24173966/raycasting-engine-rendering-creating-slight-distortion-increasing-towards-edges
            double rayDir = player->dir + atan((x - screenWidth/2.0) / distanceToSurface);

            // set tileStepX and tileStepY
            int tileStepX = 0;
            int tileStepY = 0;
            int q = quadrant(rayDir);
            if (q == 1)
            {
                tileStepX = 1;
                tileStepY = 1;
            }
            else if (q == 2)
            {
                tileStepX = -1;
                tileStepY = 1;
            }
            else if (q == 3)
            {
                tileStepX = -1;
                tileStepY = -1;
            }
            else if (q == 4)
            {
                tileStepX = 1;
                tileStepY = -1;
            }

            // calculate length until next cell is reached in x & y
            Vector cellDistance;
            if (tileStepX == 1)
                cellDistance.x = 1 - (player->pos.x - floor(player->pos.x));
            else
                cellDistance.x = player->pos.x - floor(player->pos.x);
            if (tileStepY == 1)
                cellDistance.y = 1 - (player->pos.y - floor(player->pos.y));
            else
                cellDistance.y = player->pos.y - floor(player->pos.y);
            // edge case to handle 0 distance TODO clean this up
            if (cellDistance.x == 0)
                cellDistance.x = 1;
            if (cellDistance.y == 0)
                cellDistance.y = 1;

            // calculate y & x intercept offset from player
            double yInterceptOffset = cellDistance.x * tan(rayDir);
            if (yInterceptOffset < 0) // TODO cleanup
                yInterceptOffset = yInterceptOffset * -1;
            double xInterceptOffset = cellDistance.y / tan(rayDir);
            if (xInterceptOffset < 0) // TODO cleanup
                xInterceptOffset = xInterceptOffset * -1;

            // setup rays for detecting cell walls
            Vector yIntercept;
            yIntercept.x = player->pos.x + (cellDistance.x * (double) tileStepX);
            yIntercept.y = player->pos.y + (yInterceptOffset * (double) tileStepY);
            Vector xIntercept;
            xIntercept.x = player->pos.x + (xInterceptOffset * (double) tileStepX);
            xIntercept.y = player->pos.y + (cellDistance.y * (double) tileStepY);

            // calculate ystep and xstep for ray projection
            // TODO using float here as a quick fix for precision issues
            // TODO need to check for both conditions below in hit loop in order to prevent endless loop
            float xstep = rayDir == 0 ? 0 : 1 / tan(rayDir);
            if (xstep < 0) // TODO cleanup
                xstep = xstep * -1;
            xstep = xstep * tileStepX;
            float ystep = tan(rayDir);
            if (ystep < 0) // TODO cleanup
                ystep = ystep * -1;
            ystep = ystep * tileStepY;

#ifdef GAME_DEBUG
            printf("quadrant: %d, degrees: %f, dir: %f, tan dir: %f, celld: (%f, %f), intercepts: (%f, %f), player: (%f, %f), x-intercept: (%f, %f), y-intercept: (%f, %f)\n",
                    q, to_degrees(rayDir), rayDir, 
                    tan(rayDir),
                    cellDistance.x, cellDistance.y,
                    xInterceptOffset, yInterceptOffset,
                    player->pos.x, player->pos.y,
                    xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif

            // increment ray pos until we hit wall, *or* go past map bounds
            int hit = 0;
            int i = 0;
            WallSide side; // 0 for x-intercept, 1 for y-intercept
            while (hit == 0)
            {
                // hack for bounds check TODO fix this
                if (++i > 10000) {
                    printf("Raycast error, angle: %f, rayDir: %f\n", player->dir, rayDir);
                    break;
                }
                // check for x-intercept
                if (tileStepY == 1)
                {
                    while (xIntercept.y <= yIntercept.y && hit == 0)
                    {
                        if (hit_wall(map, xIntercept, WALL_NORTH))
                        {
                            hit = 1;
                            side = WALL_NORTH;
                            break;
                        }
                        xIntercept.x += xstep;
                        xIntercept.y += tileStepY;
                    }
                }
                else
                {
                    while (xIntercept.y >= yIntercept.y && hit == 0)
                    {
                        if (hit_wall(map, xIntercept, WALL_SOUTH))
                        {
                            hit = 1;
                            side = WALL_SOUTH;
                            break;
                        }
                        xIntercept.x += xstep;
                        xIntercept.y += tileStepY;
                    }
                }

                // check for y-intercept
                if (tileStepX == 1)
                {
                    while (yIntercept.x <= xIntercept.x && hit == 0)
                    {
                        if (hit_wall(map, yIntercept, WALL_WEST))
                        {
                            hit = 1;
                            side = WALL_WEST;
                            break;
                        }
                        yIntercept.y += ystep;
                        yIntercept.x += tileStepX;
                    }
                }
                else
                {
                    while (yIntercept.x >= xIntercept.x && hit == 0)
                    {
                        if (hit_wall(map, yIntercept, WALL_EAST))
                        {
                            hit = 1;
                            side = WALL_EAST;
                            break;
                        }
                        yIntercept.y += ystep;
                        yIntercept.x += tileStepX;
                    }
                }
            }

            if (hit)
            {
                Vector rayPos;
                if (side == WALL_NORTH || side == WALL_SOUTH)
                {
                    rayPos.x = xIntercept.x;
                    rayPos.y = xIntercept.y;
                }
                else
                {
                    rayPos.x = yIntercept.x;
                    rayPos.y = yIntercept.y;
                }

#ifdef GAME_DEBUG
                printf("hit found: (%f, %f); side: %d\n", rayPos.x, rayPos.y, side);
#endif

                // calculate proportional distance (corrects for fisheye effect)
                /* double propDist = distance(rayPos, player->pos) * cos(rayDir - player->dir); */
                // more efficient way:
                // delta x = d * cos(rayDir.x), delta y = d * cos(rayDir.y)
                // which expands into:
                double propDist = cos(player->dir) * (rayPos.x - player->pos.x) + sin(player->dir) * (rayPos.y - player->pos.y);

                wallZ[x] = propDist;

                // calculate wall proportion percentage
                double proportion = 1 / propDist;
                if (proportion < 0)
                    proportion = 0;

                // calculate wall height & ypos
                double wallHeight = screenHeight * proportion;
                double y = (screenHeight - wallHeight) / 2;

                // calculate which part of texture to render
                /* Vector difference = (Vector) { rayPos.x - floor(rayPos.x), rayPos.y - floor(rayPos.y) }; */ // using vector subtraction
                /* double wallX = sqrt(difference.x*difference.x + difference.y*difference.y); */
                // more efficient:
                double wallX; // where within the wall did the ray hit
                if (side == WALL_NORTH || side == WALL_SOUTH) wallX = rayPos.x - floor(rayPos.x);
                else wallX = rayPos.y - floor(rayPos.y);

                // get the texture from the map tile
                Vector tilePos = rayPos; // TODO remove this & simplify rayPos
                if (side == WALL_SOUTH)
                    tilePos.y -= 1;
                if (side == WALL_EAST)
                    tilePos.x -= 1;
                const Tile *t = get_tile(map, tilePos.x, tilePos.y);

                if (t == NULL)
                {
#ifdef GAME_DEBUG
                    printf("Error - wall tile is null\n");
#endif
                    return 0;
                }

                const SubTexture *texture = t->texture;

                // offset from within texture (we're only rendering 1 slice of the wall)
                int textureX = wallX * texture->width;

                // draw walls on layer 1
                draw_start(1);

                // draw texture
                draw_texture(texture->atlas->texture,
                        texture->xOffset + textureX, texture->yOffset, 1, texture->height,
                        x, y, 1, wallHeight);

                // TODO add simple lighting
                /* double lighting = 1 / propDist; */
                /* if (lighting > 1) lighting = 1; */
                /* if (side == 1) lighting *= 0.75; */
                /* draw_line(x, y, x, y + wallHeight, 255, 255, 255, 100*lighting); */

                // calculate normalized rayPos from playerPos in order to multiply by distance
                /* Vector normalRay = normalize((Vector) { rayPos.x - player->pos.x, rayPos.y - player->pos.y }); */
                Vector floorStart = rayPos;
                Vector floorPos = floorStart;

                // draw floors below wall
                int yStart = (int)y + (int)wallHeight;
                for (y = yStart; y < screenHeight; y++)
                {
                    // the distance, from 1 to infinity, where infinity is middle of screen and 1 is bottom of screen
                    double currentDist = screenHeight / (2.0 * y - screenHeight); // TODO protect against divide by zero
                    double t = currentDist / propDist;

                    // guard against a weight of >1
                    if (t > 1.0) {
                        t = 1.0;
                    }

                    // using linear interpolation:
                    floorPos.x = (1 - t) * player->pos.x + t * floorStart.x;
                    floorPos.y = (1 - t) * player->pos.y + t * floorStart.y;

                    // get floor tile pos
                    Vector tilePos = floorPos;

                    // ensure we're not drawing the starting wall tile TODO remove this & cleanup rayPos
                    if ((int) tilePos.x == (int) rayPos.x &&
                        (int) tilePos.y == (int) rayPos.y)
                    {
                        if (side == WALL_NORTH)
                            tilePos.y -= 1;
                        if (side == WALL_WEST)
                            tilePos.x -= 1;
                    }

                    // get the floor tile
                    const Tile *tile = get_tile(map, tilePos.x, tilePos.y);

                    if (tile == NULL)
                    {
#ifdef GAME_DEBUG
                        printf("Error - floor tile is null\n");
#endif
                        return 0;
                    }

                    // get colors from tile's subtexture
                    Color *colors = tile->texture->pixels;

                    double floorY = (floorPos.y - floor(floorPos.y)) * tile->texture->height;
                    double floorX = (floorPos.x - floor(floorPos.x)) * tile->texture->width;

                    // hardcoded version from before - a bit better performance
                    /* const unsigned int offset = (pitch/sizeof(Uint32))*y + x; */
                    /* const unsigned int textureOffset = */ 
                    /*     (texturePartWidth*3 + (int) floorX) + ((textureSurface->pitch)/4 * (int) floorY); */
                    /* pixels[offset] = textureColors[textureOffset]; */

                    // TODO performance
                    // TODO will this work with all source pixel formats?
                    const unsigned int offset = (pitch/sizeof(Uint32))*y + x;
                    const unsigned int textureOffset = 
                        ((int) floorX) + (tile->texture->atlas->width * (int) floorY);
                    pixels[offset] = (0xFF << 24) |
                        (colors[textureOffset].r << 16) |
                        (colors[textureOffset].g << 8) |
                        (colors[textureOffset].b);
                }
            } else {
                wallZ[x] = 99999;
            }
        }

        SDL_UnlockTexture(streamTexture);

        // copy floor to ceiling (layer 1 - texture target)
        draw_start(1);
        SDL_Rect rect = (SDL_Rect) { 0, 0, screenWidth, screenHeight };
        SDL_RenderCopyEx(get_renderer(),
                streamTexture,
                &rect,
                &rect,
                0,
                NULL,
                SDL_FLIP_VERTICAL);
    }
    // end draw the floor and the walls

    // draw enemies
    {
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
            enemy->angle = acos(dot_product(normPlayer, normEnemy));

            // dist is euclidian distance (from player to enemy)
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
            Sprite enemy = enemies[i];
            Vector enemyPos = enemy.pos;

            // calculate which side of screen
            int side = 1; // right side
            if ((player->dir <= M_PI && (cos(player->dir)*enemy.distY + player->pos.x < enemyPos.x)) ||
                    (player->dir > M_PI && (cos(player->dir)*enemy.distY + player->pos.x > enemyPos.x)))
                side = -1; // left side

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
            double spriteX = (midX - enemyHeight/2) + side * (enemy.distX*distanceToSurface/enemy.distY);
            double spriteY = midY - enemyHeight/2;

            if (enemy.angle <= player->fov)
            {
                // draw texture column by column, only if z value higher than wallZ
                int textureOffsetX = enemy.texture->xOffset;
                int textureOffsetY = enemy.texture->yOffset;
                double step = enemyHeight / 61;
                for (int x = 0; x < 61; ++x)
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