#include "draw.h"
#include "game.h"

#include "SDL.h"
#include <stdlib.h>

#define MAX_FPS 30

int flag_exists(const char *flag, int argc, char **argv)
{
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], flag) == 0)
            return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Error initializing SDL\n");

        return 1;
    }

    // default width & height
    int width = 800;
    int height = 600;
    int flags = 0;

    SDL_DisplayMode display;
    if (!(flag_exists("-w", argc, argv) || flag_exists("--windowed", argc, argv)) &&
            SDL_GetCurrentDisplayMode(0, &display) == 0)
    {
        width = display.w;
        height = display.h;
        flags = SDL_WINDOW_FULLSCREEN;
    }

    SDL_Window *win = SDL_CreateWindow("Raycasting Demo",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, flags);

    if (win == NULL)
    {
        printf("Error creating window\n");

        return 1;
    }

    // capture mouse within SDL window
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // some useful flags SDL_RENDERER_SOFTWARE or SDL_RENDERER_ACCELERATED
    // NOTE: looks like accelerated is inferred by default
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

    // draw background layer
    draw_start(0);
    draw_gradient(0, 0, width, height / 2, 1000,
            122, 122, 122, 255,
            0, 0, 0, 255);
    draw_gradient(0, height / 2, width, height / 2, 1000,
            0, 0, 0, 255,
            61, 61, 61, 255);

    // game loop
    init_game();
    while (tick_game())
    {
        // calculate FPS
        unsigned int time = SDL_GetTicks();
        unsigned int diff = time - lastTime;
        double fps = 1 / (diff / 1000.0f);
        // sleep if FPS > MAX_FPS
        while (fps > MAX_FPS)
        {
            diff = SDL_GetTicks() - lastTime;
            fps = 1 / (diff / 1000.0f);
        }

        if (flag_exists("--stats", argc, argv) || flag_exists("-s", argc, argv))
        {
            // update window title with FPS
            char title[100];
            sprintf(title, "Raycasting Demo (FPS: %f, Pos: (%f, %f), Angle: %f)", fps, get_player().pos.x, get_player().pos.y, to_degrees(get_player().dir));
            SDL_SetWindowTitle(win, title);
        }

        // update lastTime for next iteration
        lastTime = time;
    }

    /* SDL_DestroyTexture(tex); */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
