#include "draw.h"

#include "SDL.h"

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error initializing SDL\n");

        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Raycasting Demo",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            800, 600, 0);

    if (win == NULL) {
        printf("Error creating window\n");

        return 1;
    }

    // some useful flags SDL_RENDERER_SOFTWARE or SDL_RENDERER_ACCELERATED
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, 0);

    if (renderer == NULL) {
        printf("Error creating renderer\n");

        return 1;
    }

    // initialize our drawing
    draw_init(renderer);
    draw_rect(50, 50, 400, 400,
            0, 255, 255, 255);
    draw_update();

    getchar();

    /* SDL_DestroyTexture(tex); */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
