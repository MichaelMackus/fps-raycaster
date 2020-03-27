#include "engine/entity.h"
#include "engine/map.h"
#include "input.h"

#include "SDL.h"

int handle_input(Map *map)
{
    int playing = 1;

    Player *player = get_player();

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            playing = 0;

        if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_q:
                    playing = 0;
                    break;
            }
#ifdef GAME_DEBUG
            printf("X: %f, Y: %f, Dir: %f\n", player->pos.x, player->pos.y, player->dir);
#endif
        }
    }

    // TODO use this to get mouse pos:
    /* SDL_GetRelativeMouseState(&relx, &rely); */

    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    if(keystates[SDL_SCANCODE_W])
    {
        // walk forward in unit circle (unit circle x = cos, y = sin)
        player->pos.x += cos(player->dir) / 10;
        player->pos.y += sin(player->dir) / 10;
        if (!is_passable(map, player->pos.x, player->pos.y))
        {
            player->pos.x -= cos(player->dir) / 10;
            player->pos.y -= sin(player->dir) / 10;
        }
    }
    if(keystates[SDL_SCANCODE_S])
    {
        // walk backward in unit circle (unit circle x = cos, y = sin)
        player->pos.x -= cos(player->dir) / 10;
        player->pos.y -= sin(player->dir) / 10;
        if (!is_passable(map, player->pos.x, player->pos.y))
        {
            player->pos.x += cos(player->dir) / 10;
            player->pos.y += sin(player->dir) / 10;
        }
    }
    if (keystates[SDL_SCANCODE_A])
    {
        // turn left
        player->dir = rotate(player->dir, -0.05);
    }
    if (keystates[SDL_SCANCODE_D])
    {
        // turn right
        player->dir = rotate(player->dir, 0.05);
    }
    if (keystates[SDL_SCANCODE_LCTRL] || keystates[SDL_SCANCODE_RCTRL])
    {
        // shoot bullet
        player->shooting = 1;
    }

    return playing;
}

