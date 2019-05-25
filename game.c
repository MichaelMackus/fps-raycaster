#include "draw.h"
#include "game.h"

#include "SDL.h"

#define MAP_WIDTH 19
#define MAP_HEIGHT 19
#define MAP_MAX_DISTANCE MAP_WIDTH*MAP_WIDTH + MAP_HEIGHT*MAP_HEIGHT

static Player player;

Player get_player()
{
    return player;
}

static char map[MAP_HEIGHT][MAP_WIDTH] =
    {"###################",
     "#.................#",
     "#.................#",
     "#........#........#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#......#####......#",
     "#......#...#......#",
     "#......#.@.#......#",
     "#......#...#......#",
     "#......##.##......#",
     "#.................#",
     "#.......#.#.......#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "###################"};

int screenWidth;
int screenHeight;

// our texture image
static SDL_Texture *texture;
int textureWidth;
int textureHeight;

int init_game()
{
    player.pos.x = 9.0f;
    player.pos.y = 9.0f;
    player.dir = 0.0f;
    player.fov = to_radians(90);

    get_screen_dimensions(&screenWidth, &screenHeight);

    texture = load_texture("wolftextures.png");

    if (texture == NULL)
        return 1;

    return SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
}

typedef enum {
    WALL_NORTH,
    WALL_EAST,
    WALL_WEST,
    WALL_SOUTH
} WallSide;

// whether vector hits wall side
int hit_wall(Vector pos, WallSide side)
{
    if (side == WALL_SOUTH)
    {
        pos.y -= 1;
    }

    if (side == WALL_EAST)
    {
        pos.x -= 1;
    }

    return (map[(int) pos.y][(int) pos.x] == '#');
}

int tick_game()
{
    int playing = 1;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            playing = 0;

        if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_q:
                    playing = 0;
                    break;
            }
#ifdef GAME_DEBUG
            printf("X: %f, Y: %f, Dir: %f\n", player.pos.x, player.pos.y, player.dir);
#endif
        }
    }

    // TODO use this to get mouse pos:
    /* SDL_GetRelativeMouseState(&relx, &rely); */

    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    if(keystates[SDL_SCANCODE_W])
    {
        // walk forward in unit circle (unit circle x = cos, y = sin)
        player.pos.x += cos(player.dir) / 5;
        player.pos.y += sin(player.dir) / 5;
        if (map[(int) player.pos.y][(int) player.pos.x] == '#') {
            player.pos.x -= cos(player.dir) / 5;
            player.pos.y -= sin(player.dir) / 5;
        }
    }
    if(keystates[SDL_SCANCODE_S])
    {
        // walk backward in unit circle (unit circle x = cos, y = sin)
        player.pos.x -= cos(player.dir) / 5;
        player.pos.y -= sin(player.dir) / 5;
        if (map[(int) player.pos.y][(int) player.pos.x] == '#') {
            player.pos.x += cos(player.dir) / 5;
            player.pos.y += sin(player.dir) / 5;
        }
    }
    if (keystates[SDL_SCANCODE_A])
    {
        // turn left
        player.dir = rotate(player.dir, -0.05f);
    }
    if (keystates[SDL_SCANCODE_D])
    {
        // turn right
        player.dir = rotate(player.dir, 0.05f);
    }

    // start drawing on layer 1
    draw_start(1);

    // calculate distance from player to screen - this will be screenWidth/2 for 90 degree FOV
    double distanceToSurface = (screenWidth/2.0f) / tan(player.fov/2);

    // don't re-draw floors past this y-value
    int highestFloorY = screenHeight;

    // detect which squares the player can see, and draw them proportionally to distance
    for (int x = 0; x <= screenWidth; ++x)
    {
        // calculate ray direction
        // double rayDir = player.dir - (player.fov/2) + x * (player.fov/screenWidth); // generates distortion towards edges
        // fix for increased distortion towards screen edges
        // see: https://stackoverflow.com/questions/24173966/raycasting-engine-rendering-creating-slight-distortion-increasing-towards-edges
        double rayDir = player.dir + atan((x - screenWidth/2.0f) / distanceToSurface);

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
            cellDistance.x = 1 - (player.pos.x - floor(player.pos.x));
        else
            cellDistance.x = player.pos.x - floor(player.pos.x);
        if (tileStepY == 1)
            cellDistance.y = 1 - (player.pos.y - floor(player.pos.y));
        else
            cellDistance.y = player.pos.y - floor(player.pos.y);
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
        yIntercept.x = player.pos.x + (cellDistance.x * (double) tileStepX);
        yIntercept.y = player.pos.y + (yInterceptOffset * (double) tileStepY);
        Vector xIntercept;
        xIntercept.x = player.pos.x + (xInterceptOffset * (double) tileStepX);
        xIntercept.y = player.pos.y + (cellDistance.y * (double) tileStepY);

        // calculate ystep and xstep for ray projection
        double xstep = rayDir == 0 ? 0 : 1 / tan(rayDir);
        if (xstep < 0) // TODO cleanup
            xstep = xstep * -1;
        xstep = xstep * tileStepX;
        double ystep = tan(rayDir);
        if (ystep < 0) // TODO cleanup
            ystep = ystep * -1;
        ystep = ystep * tileStepY;

#ifdef GAME_DEBUG
        printf("quadrant: %d, degrees: %f, dir: %f, tan dir: %f, celld: (%f, %f), intercepts: (%f, %f), player: (%f, %f), x-intercept: (%f, %f), y-intercept: (%f, %f)\n",
                q, to_degrees(rayDir), rayDir, 
                tan(rayDir),
                cellDistance.x, cellDistance.y,
                xInterceptOffset, yInterceptOffset,
                player.pos.x, player.pos.y,
                xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif

        // increment ray pos until we hit wall, *or* go past map bounds
        int hit = 0;
        WallSide side; // 0 for x-intercept, 1 for y-intercept
        while (hit == 0) // TODO bounds checking
        {
            // check for x-intercept
            if (tileStepY == 1)
            {
                while (xIntercept.y <= yIntercept.y && hit == 0)
                {
                    if (hit_wall(xIntercept, WALL_NORTH))
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
                    if (hit_wall(xIntercept, WALL_SOUTH))
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
                    if (hit_wall(yIntercept, WALL_WEST))
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
                    if (hit_wall(yIntercept, WALL_EAST))
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
            /* double propDist = distance(rayPos, player.pos) * cos(rayDir - player.dir); */
            // more efficient way:
            // delta x = d * cos(rayDir.x), delta y = d * cos(rayDir.y)
            // which expands into:
            double propDist = cos(player.dir) * (rayPos.x - player.pos.x) + sin(player.dir) * (rayPos.y - player.pos.y);

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

            int texturePartWidth = 64; // width of a single texture within texture file
            int textureX = wallX * texturePartWidth;

            // draw texture
            draw_texture(texture,
                    texturePartWidth + textureX, 0, 1, textureHeight,
                    x, y, 1, wallHeight);

            // TODO add simple lighting
            /* double lighting = 1 / propDist; */
            /* if (lighting > 1) lighting = 1; */
            /* if (side == 1) lighting *= 0.75; */
            /* draw_line(x, y, x, y + wallHeight, 255, 255, 255, 100*lighting); */

            // calculate normalized rayPos from playerPos in order to multiply by distance
            Vector normalRay = normalize((Vector) { rayPos.x - player.pos.x, rayPos.y - player.pos.y });
            Vector floorPos = rayPos;

            // draw floors below wall
            int yStart = y + wallHeight;
            int texturePartHeight = 64; // height of a single texture within texture file
            for (y = yStart; y < screenHeight; ++y)
            {
                // the distance, from 1 to infinity, where infinity is middle of screen and 1 is bottom of screen
                double currentDist = screenHeight / (2.0 * y - screenHeight);
                double t = currentDist / propDist; // weight factor

                // using normalized vector: (too slow, uses sqrt)
                /* double length = distance(floorPos, player.pos); */
                /* floorPos.x += normalRay.x * length; // comes out squished */
                /* floorPos.y += normalRay.y * length; */

                // using linear interpolation:
                floorPos.x = (1 - t) * player.pos.x + t * rayPos.x;
                floorPos.y = (1 - t) * player.pos.y + t * rayPos.y;

                double floorY = (floorPos.y - floor(floorPos.y)) * texturePartHeight;
                double floorX = (floorPos.x - floor(floorPos.x)) * texturePartWidth;

                // TODO perhaps draw with streaming access to texture, or use SDL_Surface
                draw_texture(texture,
                        texturePartWidth*3 + floorX, floorY, 1, 1,
                        x, y, 1, 1);
            }
        }
    }

    // finish drawing to layer 1
    draw_update(1);

    return playing;
}
