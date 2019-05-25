#include "draw.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

// textures array initialized by layers
static SDL_Texture **textures;
static int textures_size = 0;

// current texture being drawn to
static SDL_Texture *texture;

// highest texture layer initialized
static int layer_initialized = -1;

int draw_init(SDL_Window *win, SDL_Renderer *r)
{
    if (r == NULL)
        return 1;

    renderer = r;
    window = win;

    return 0;
}

SDL_Renderer* get_renderer()
{
    return renderer;
}

int draw_init_layer(int layer, int colorMode, int accessMode, int alphaBlend)
{
    if (layer >= textures_size)
    {
        // realloc textures
        int new_size = layer + 1;
        SDL_Texture **tmp = realloc(textures, sizeof(*textures) * new_size);

        if (tmp == NULL)
            return 1;

        // ensure newly allocated memory is initialized to NULL
        memset(tmp + textures_size, 0, sizeof(*textures) * (new_size - textures_size));

        textures = tmp;
        textures_size = new_size;
    }

    if (textures[layer] == NULL)
    {
        int w, h;
        get_screen_dimensions(&w, &h);

        // initialize texture, or error
        textures[layer] = SDL_CreateTexture(renderer,
                colorMode, accessMode, w, h);

        if (textures[layer] == NULL)
            return 1;
    }

    texture = textures[layer];
    SDL_SetRenderTarget(renderer, texture);

    if (alphaBlend)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    layer_initialized = layer;
}

int draw_start(int layer)
{
    // only initialize layer first time
    if (layer_initialized < layer) draw_init_layer(layer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1);

    texture = textures[layer];
    SDL_SetRenderTarget(renderer, texture);

    return 0;
}

int draw_update(int layer)
{
    SDL_SetRenderTarget(renderer, NULL);

    // copy layers in order into renderer
    if (textures_size > 0)
    {
        int w, h;
        get_screen_dimensions(&w, &h);
        SDL_Rect screen = { 0, 0, w, h };

        for (int i = 0; i < textures_size; i++)
        {
            SDL_Texture *curLayer = textures[i];

            if (curLayer == NULL)
                continue;

            SDL_RenderCopy(renderer, curLayer, NULL, NULL);
        }
    }

    // reset initialized layer
    layer_initialized = - 1;

    SDL_RenderPresent(renderer);

    return 0;
}

SDL_Texture* get_texture(int layer)
{
    return textures[layer];
}

SDL_Texture* load_texture(const char *filename)
{
    SDL_Surface *surface = IMG_Load(filename);

    if (surface == NULL)
        return NULL;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);

    return texture;
}

int draw_texture(SDL_Texture *texture,
        int x1, int y1, int w1, int h1,
        int x2, int y2, int w2, int h2)
{
    SDL_Rect from = { x1, y1, w1, h1 };
    SDL_Rect to = { x2, y2, w2, h2 };

    return SDL_RenderCopy(renderer,
            texture,
            &from,
            &to);
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

int draw_gradient(int x, int y, int w, int h, int steps,
        Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
        Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2)
{
    // amount of pixels each step through gradient represents
    double gradientStep = (double) h / (double) steps;
    // amount of color we add for each step through gradient
    double r = (double) (r2 - r1) / (double) steps;
    double g = (double) (g2 - g1) / (double) steps;
    double b = (double) (b2 - b1) / (double) steps;
    double a = (double) (a2 - a1) / (double) steps;

    // loop through gradient top to bottom in steps
    for (int i = 0; i < steps; i ++)
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
        {
            SDL_Rect rect = (SDL_Rect) { x, y1, w, y2 - y1 };
            SDL_RenderDrawRect(renderer, &rect);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    return 0;
}
