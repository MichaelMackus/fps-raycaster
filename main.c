#include "engine/raycast.h"
#include "engine/entity.h"
#include "game.h"
#include "input.h"
#include "pixelgfx/gfx.h"
#include "pixelgfx/sdl.h"

#include "SDL.h"
#include "SDL_image.h"
#include <stdlib.h>

#define MAX_FPS 999

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
    int width = 1920, height = 1080;
    int displayW = width, displayH = height;
    int flags = 0;

    SDL_DisplayMode display;
    if (!(flag_exists("-w", argc, argv) || flag_exists("--windowed", argc, argv)) &&
            SDL_GetCurrentDisplayMode(0, &display) == 0)
    {
        displayW = display.w;
        displayH = display.h;
        flags = SDL_WINDOW_FULLSCREEN;
    }

    SDL_Window *win = pixel_sdl_new_window("Raycasting Demo", displayW, displayH, flags);

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

    // initialize opengl buffer
    char *buffer = pixel_gfx_init(width, height, GL_RGBA);
    memset(buffer, 0, height * width * 4);

    // capture mouse within SDL window
    SDL_SetRelativeMouseMode(SDL_TRUE);

    unsigned int lastTime;
    lastTime = SDL_GetTicks();

        printf("%d\n", win);

    if (init_game(width, height) == 1 || init_raycast(width, height) == 1)
    {
        printf("Error initializing game.\n");

        return 1;
    }

    // load our game map
    Map *map = load_map("map.txt");

    // game loop
    while (handle_input(map))
    {
        // update screen data
        if (!do_raycast(map, width, height, buffer)) break;

        // draw buffer
        pixel_gfx_update(buffer, 0, 0, 0, 255);
        pixel_sdl_render(win);

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
            printf("%s\n", title);
            SDL_SetWindowTitle(win, title);
        }

        // update lastTime for next iteration
        lastTime = time;
    }

    destroy_raycast();

    IMG_Quit();
    pixel_sdl_end(win);
    pixel_gfx_end(buffer);

    return 0;
}
