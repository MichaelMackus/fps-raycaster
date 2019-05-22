#include "draw.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

static SDL_Texture *texture;

int draw_init(SDL_Window *win, SDL_Renderer *r)
{
    if (r == NULL)
        return 1;

    renderer = r;
    window = win;

    // get width & height for texture
    int w;
    int h;
    SDL_GetWindowSize(window, &w, &h);

    // initialize texture, or error
    // TODO move this to draw_start and index into array by layer
    texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

    if (texture == NULL)
        return 1;

    return 0;
}

int draw_start(int layer)
{
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    return 0;
}

int draw_update(int layer)
{
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    return 0;
}

void get_screen_dimensions(int *w, int *h)
{
    SDL_GetWindowSize(window, w, h);
}

int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    SDL_Rect rect = (SDL_Rect) { x, y, w, h };
    SDL_RenderDrawRect(renderer, &rect);
    SDL_RenderFillRect(renderer, &rect);

    return 0;
}

int draw_line(int x1, int y1, int x2, int y2,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

    return 0;
}

int draw_gradient(int x, int y, int w, int h,
        Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
        Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2)
{
    // amount of pixels each step through gradient represents
    int gradientStep = h / GRADIENT_STEPS;
    // amount of color we add for each step through gradient
    float r = (r2 - r1) / GRADIENT_STEPS;
    float g = (g2 - g1) / GRADIENT_STEPS;
    float b = (b2 - b1) / GRADIENT_STEPS;
    float a = (a2 - a1) / GRADIENT_STEPS;

    // loop through gradient top to bottom in GRADIENT_STEPS
    for (int i = 0; i < GRADIENT_STEPS; i ++)
    {
        SDL_SetRenderDrawColor(renderer, r1 + r*i, g1 + g*i, b1 + b*i, a1 + a*i);

        int y1 = y + (gradientStep * i);
        int y2 = y + (gradientStep * (i+1));

        if (w == 1)
            SDL_RenderDrawLine(renderer,
                    x,
                    y1,
                    x,
                    y2);
        else
            // TODO implement rectangle gradients
            SDL_RenderDrawLine(renderer,
                    x,
                    y1,
                    x,
                    y2);
    }

    return 0;
}
