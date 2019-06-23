#include "engine/draw.h"
#include "engine/raycast.h"
#include "engine/entity.h"
#include "game.h"
#include "input.h"

#include "SDL.h"
#include "SDL_image.h"
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

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("Error initializing PNG image loading.\n");

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

    // turn on alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // initialize our drawing
    if (draw_init(win, renderer) != 0)
    {
        printf("Error initializing\n");

        return 1;
    }

    unsigned int lastTime;
    lastTime = SDL_GetTicks();

    if (init_game() == 1 || init_raycast() == 1)
    {
        printf("Error initializing game.\n");

        return 1;
    }

    // game loop
    while (handle_input())
    {
        // update screen data
        if (!raycast()) break;

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
            Player *player = get_player();
            sprintf(title, "Raycasting Demo (FPS: %f, Pos: (%f, %f), Angle: %f)", fps, player->pos.x, player->pos.y, to_degrees(player->dir));
            SDL_SetWindowTitle(win, title);
        }

        // update lastTime for next iteration
        lastTime = time;
    }

    destroy_raycast();
    draw_free();

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
