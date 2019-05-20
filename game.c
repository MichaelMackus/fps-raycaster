#include "draw.h"
#include "game.h"
#include "stdinc.h"

#include "SDL.h"

#define MAP_WIDTH 19
#define MAP_HEIGHT 19

#define DEBUG

typedef struct {
    Vector pos; // player position
    float dir; // angle player is facing (in radians)
    float fov; // field of view (in radians)
} Player;

static Player player;

static char map[MAP_HEIGHT][MAP_WIDTH] =
    {"###################",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#........@........#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "#.................#",
     "###################"};

int screenWidth;
int screenHeight;

void init_game()
{
    player.pos.x = 9.0f;
    player.pos.y = 9.0f;
    player.dir = 0.0f;
    player.fov = to_radians(66); // FOV is 90 degrees (math.pi / 2 in radians)

    get_screen_dimensions(&screenWidth, &screenHeight);
}

int tick_game()
{
    int playing = 1;
    int debug = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            playing = 0;

        if (e.type == SDL_KEYDOWN)
        {
            debug = 1;
            switch (e.key.keysym.sym)
            {
                case SDLK_w:
                    // walk forward in unit circle (unit circle x = cos, y = sin)
                    player.pos.x += cos(player.dir);
                    player.pos.y += sin(player.dir);
                    break;

                case SDLK_s:
                    // walk backward in unit circle (unit circle x = cos, y = sin)
                    player.pos.x -= cos(player.dir);
                    player.pos.y -= sin(player.dir);
                    break;

                case SDLK_a:
                    // turn left
                    player.dir -= 0.1f;
                    break;

                case SDLK_d:
                    // turn right
                    player.dir += 0.1f;
                    break;

                case SDLK_q:
                    playing = 0;
                    break;
            }
            printf("X: %f, Y: %f, Dir: %f\n", player.pos.x, player.pos.y, player.dir);
        }
    }

    clear_rects();

    // detect which squares the player can see, and draw them proportionally to distance
    for (int x = 0; x <= screenWidth; ++x)
    {
        // calculate ray direction
        float rayDir = player.dir - (player.fov/2) + x * (player.fov/screenWidth);

        // set tileStepX and tileStepY
        int tileStepX = 0;
        int tileStepY = 0;
        int q = quadrant(to_degrees(rayDir));
        if (q == 1) {
            tileStepX = 1;
            tileStepY = 1;
        } else if (q == 2) {
            tileStepX = -1;
            tileStepY = 1;
        } else if (q == 3) {
            tileStepX = -1;
            tileStepY = -1;
        } else if (q == 4) {
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
        // TODO handle when rayDir is close to 360 (x & yInterceptOffset = 0)
        float yInterceptOffset = cellDistance.x * tan(rayDir);
        if (yInterceptOffset < 0) // TODO cleanup
            yInterceptOffset = yInterceptOffset * -1;
        /* if (yInterceptOffset == 0) */
        /* { */
        /*     printf("yInterceptOffset == 0\n"); */
        /*     exit(1); */
        /* } */

        float xInterceptOffset = cellDistance.y / tan(rayDir);
        if (xInterceptOffset < 0) // TODO cleanup
            xInterceptOffset = xInterceptOffset * -1;
        /* if (xInterceptOffset == 0) */
        /* { */
        /*     printf("xInterceptOffset == 0\n"); */
        /*     exit(1); */
        /* } */

        // setup rays for detecting cell walls
        Vector yIntercept;
        yIntercept.x = player.pos.x + (cellDistance.x * tileStepX);
        yIntercept.y = player.pos.y + (yInterceptOffset * tileStepY);
        Vector xIntercept;
        xIntercept.x = player.pos.x + (xInterceptOffset * tileStepX);
        xIntercept.y = player.pos.y + (cellDistance.y * tileStepY);
        // calculate ystep and xstep for ray projection
        float xstep = 1 / tan(rayDir);
        if (xstep < 0) // TODO cleanup
            xstep = xstep * -1;
        xstep = xstep * tileStepX;
        float ystep = tan(rayDir);
        if (ystep < 0) // TODO cleanup
            ystep = ystep * -1;
        ystep = ystep * tileStepY;

#ifdef DEBUG
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
        int side; // 0 for x-intercept, 1 for y-intercept
        while (hit == 0) // TODO bounds checking */
        {
            // check for x-intercept
            if (tileStepY == 1)
            {
                while (xIntercept.y < yIntercept.y && hit == 0)
                {
#ifdef DEBUG
                    printf("X x-intercept: (%f, %f), y-intercept: (%f, %f)\n", xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif
                    if (map[(int) xIntercept.y][(int) xIntercept.x] == '#')
                    {
                        hit = 1;
                        side = 0;
                        break;
                    }
                    xIntercept.x += xstep;
                    xIntercept.y += tileStepY;
                }
            }
            else
            {
                while (xIntercept.y > yIntercept.y && hit == 0)
                {
#ifdef DEBUG
                    printf("X x-intercept: (%f, %f), y-intercept: (%f, %f)\n", xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif
                    if (map[(int) xIntercept.y][(int) xIntercept.x] == '#')
                    {
                        hit = 1;
                        side = 0;
                        break;
                    }
                    xIntercept.x += xstep;
                    xIntercept.y += tileStepY;
                }
            }

            // check for y-intercept
            if (tileStepX == 1)
            {
                while (yIntercept.x < xIntercept.x && hit == 0)
                {
#ifdef DEBUG
                    printf("Y x-intercept: (%f, %f), y-intercept: (%f, %f)\n", xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif
                    if (map[(int) yIntercept.y][(int) yIntercept.x] == '#')
                    {
                        hit = 1;
                        side = 1;
                        break;
                    }
                    yIntercept.y += ystep;
                    yIntercept.x += tileStepX;
                }
            }
            else
            {
                while (yIntercept.x > xIntercept.x && hit == 0)
                {
#ifdef DEBUG
                    printf("Y x-intercept: (%f, %f), y-intercept: (%f, %f)\n", xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif
                    if (map[(int) yIntercept.y][(int) yIntercept.x] == '#')
                    {
                        hit = 1;
                        side = 1;
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
            if (side == 0)
            {
                rayPos.x = xIntercept.x;
                rayPos.y = xIntercept.y;
            }
            else
            {
                rayPos.x = yIntercept.x;
                rayPos.y = yIntercept.y;
            }

            // calculate proportional distance (corrects for fisheye effect)
            /* float propDist = distance(rayPos, player.pos) * cos(rayDir - player.dir); */
            // more efficient way:
            // delta x = d * cos(rayDir.x), delta y = d * cos(rayDir.y)
            // which expands into:
            float propDist = cos(player.dir) * (rayPos.x - player.pos.x) + sin(player.dir) * (rayPos.y - player.pos.y);

            // calculate wall proportion percentage
            float proportion = 1 / propDist;
            if (proportion < 0)
                proportion = 0;

            // calculate wall height & ypos
            float wallHeight = screenHeight * proportion;
            float y = (screenHeight - wallHeight) / 2;

            // TODO lighting
            draw_rect(x, y, 1, wallHeight, 255, 255, 255, 255);
        }
    }

    draw_update();
    
    return playing;
}
