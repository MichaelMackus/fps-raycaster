#include "draw.h"

#include "SDL.h"
#include <stdlib.h>

#define MAP_WIDTH 100
#define MAP_HEIGHT 100

void gameloop()
{
    int quit = 0;
    float x = 0.0f;
    float y = 0.0f;
    float rectX = 50.0f;
    float rectY = 0.0f;
    int rectW = 700;
    int rectH = 500;

    while (!quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = 1;

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
                        quit = 1;
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
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Error initializing SDL\n");

        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Raycasting Demo",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            800, 600, 0);

    if (win == NULL)
    {
        printf("Error creating window\n");

        return 1;
    }

    // some useful flags SDL_RENDERER_SOFTWARE or SDL_RENDERER_ACCELERATED
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, 0);

    if (renderer == NULL)
    {
        printf("Error creating renderer\n");

        return 1;
    }

    // initialize our drawing
    if (draw_init(win, renderer) != 0)
    {
        printf("Error initializing\n");

        return 1;
    }

    gameloop();

    /* SDL_DestroyTexture(tex); */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
