#include "game.h"
#include "input.h"

#include "SDL.h"

int handle_input()
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

    Player *player = get_player();
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    if(keystates[SDL_SCANCODE_W])
    {
        // walk forward in unit circle (unit circle x = cos, y = sin)
        player->pos.x += cos(player->dir) / 10;
        player->pos.y += sin(player->dir) / 10;
        if (get_tile(player->pos.x, player->pos.y) == '#') {
            player->pos.x -= cos(player->dir) / 10;
            player->pos.y -= sin(player->dir) / 10;
        }
    }
    if(keystates[SDL_SCANCODE_S])
    {
        // walk backward in unit circle (unit circle x = cos, y = sin)
        player->pos.x -= cos(player->dir) / 10;
        player->pos.y -= sin(player->dir) / 10;
        if (get_tile(player->pos.x, player->pos.y) == '#') {
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

    return playing;
}

