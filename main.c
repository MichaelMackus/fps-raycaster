#include "draw.h"
#include "game.h"

#include "SDL.h"
#include <stdlib.h>

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

    unsigned int lastTime;
    lastTime = SDL_GetTicks();

    // game loop
    init_game();
    while (tick_game())
    {
        // calculate FPS
        unsigned int time = SDL_GetTicks();
        unsigned int diff = time - lastTime;
        float fps = 60 / ((float) diff / 1000);
        lastTime = time; // update lastTime for next iteration
        // update window title with FPS
        char title[100];
        sprintf(title, "Raycasting Demo (FPS: %f, Pos: (%f, %f), Angle: %f)", fps, get_player().pos.x, get_player().pos.y, to_degrees(get_player().dir));
        SDL_SetWindowTitle(win, title);
    }

    /* SDL_DestroyTexture(tex); */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
