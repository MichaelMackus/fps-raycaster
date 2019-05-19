#include "draw.h"
#include "game.h"

#include "SDL.h"

#define MAP_WIDTH 100
#define MAP_HEIGHT 100

static float x = 0.0f;
static float y = 0.0f;
static float rectX = 50.0f;
static float rectY = 0.0f;
static int rectW = 700;
static int rectH = 500;

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
                    // "zoom in" rectangle
                    x += 1.0f;
                    break;

                case SDLK_s:
                    // "zoom out" rectangle
                    x -= 1.0f;
                    break;

                case SDLK_q:
                    playing = 0;
                    break;
            }
        }
    }

    draw_clear_rects();

    // draw rectangle center of screen with depth
    int distance = abs(rectX - x);
    float proportion = 1 - ((float) distance / (float) MAP_WIDTH);
    if (proportion < 0)
        proportion = 0;
    draw_rect(50, 50,
            (int) rectW * proportion, (int) rectH * proportion,
            0, 255, 255, 255);

    draw_update();
    
    return playing;
}
