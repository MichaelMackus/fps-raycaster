#include "draw.h"
#include "game.h"
#include "stdinc.h"

#include "SDL.h"

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

typedef struct {
    Vector pos; // player position
    float dir; // angle player is facing (in radians)
    float fov; // field of view (in radians)
} Player;

static Player player;

static char map[MAP_HEIGHT][MAP_WIDTH] =
    {"####################",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "#..................#",
     "####################"};

int screenWidth;
int screenHeight;

void init_game()
{
    player.pos.x = 10.0f;
    player.pos.y = 10.0f;
    player.dir = 0.0f;
    player.fov = to_radians(90); // FOV is 90 degrees (math.pi / 2 in radians)

    get_screen_dimensions(&screenWidth, &screenHeight);
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

    // TODO detect which squares the player can see, and draw them proportionally to distance
    for (int x = 0; x < screenWidth; ++x)
    {
        Vector rayPos;
        rayPos.x = player.pos.x;
        rayPos.y = player.pos.y;

        // calculate ray direction
        float rayDir = player.dir - (player.fov/2) + x * (player.fov/screenWidth);

        // increment ray pos until we hit wall, *or* go past map bounds
        int hit = 0;
        int side;
        while (rayPos.x > 0.0f && rayPos.x < (float) MAP_WIDTH &&
                rayPos.y > 0.0f && rayPos.y < (float) MAP_HEIGHT)
        {
            if (map[(int) rayPos.y][(int) rayPos.x] == '#')
            {
                // TODO hit a wall, detect what side of the wall
                hit = 1;
                break;
            }

            // increment rayPos along rayDir, TODO do we need to move in less increments?
            rayPos.x += cos(rayDir);
            rayPos.y += sin(rayDir);
        }

        if (hit)
        {
            // calculate wall proportion percentage
            float dist = distance(player.pos, rayPos);
            float proportion = 1 - (dist / (float) MAP_WIDTH);
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
