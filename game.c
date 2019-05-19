#include "draw.h"
#include "game.h"

#include "math.h"
#include "SDL.h"

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

typedef struct {
    float x; // position in x
    float y; // position in y
    float dir; // angle player is facing
    int fov; // field of view
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
    player.x = 10.0f;
    player.y = 10.0f;
    player.dir = 0.0f;
    player.fov = 90;

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
                    player.x += cos(player.dir);
                    player.y += sin(player.dir);
                    break;

                case SDLK_s:
                    // walk backward in unit circle (unit circle x = cos, y = sin)
                    player.x -= cos(player.dir);
                    player.y -= sin(player.dir);
                    break;

                case SDLK_a:
                    // turn left
                    player.dir -= 1;
                    break;

                case SDLK_d:
                    // turn right
                    player.dir = 1;
                    break;

                case SDLK_q:
                    playing = 0;
                    break;
            }
            printf("X: %f, Y: %f, Dir: %f\n", player.x, player.y, player.dir);
        }
    }

    clear_rects();

    // TODO detect which squares the player can see, and draw them proportionally to distance
    for (int x = 0; x < screenWidth; ++x)
    {
        
    }

    /* int distance = abs(rectX - x); */
    /* float proportion = 1 - ((float) distance / (float) MAP_WIDTH); */
    /* if (proportion < 0) */
    /*     proportion = 0; */
    /* draw_rect(50, 50, */
    /*         (int) rectW * proportion, (int) rectH * proportion, */
    /*         0, 255, 255, 255); */

    draw_update();
    
    return playing;
}
