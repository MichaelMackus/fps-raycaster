#include "draw.h"

static SDL_Renderer *renderer;

int draw_init(SDL_Renderer *renderer)
{
    if (renderer == NULL)
        return 1;

    renderer = renderer;

    return 0;
}

int draw_update()
{
    //First clear the renderer
    SDL_RenderClear(renderer);
    //TODO Draw the texture
    /* SDL_RenderCopy(ren, tex, NULL, NULL); */
    //Update the screen
    SDL_RenderPresent(renderer);
    //Take a quick break after all that hard work
    SDL_Delay(1000);

    return 0;
}

int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if (renderer == NULL) 
        return 1;

    SDL_SetRenderDrawColor(renderer,
            r, g, b, a);

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_RenderFillRect(renderer,
            &rect);

    return 0;

    // TODO 
    texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1024, 768);

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_RenderDrawRect(renderer,&r);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
