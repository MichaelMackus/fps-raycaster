#include "draw.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

static SDL_Texture *texture;

// represent a colored rectangle in SDL
typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_Rect rect;
} ColoredRectangle;

// represent a colored line in SDL
typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_Point from;
    SDL_Point to;
} ColoredLine;

// represent a gradient in SDL
typedef struct {
    Uint8 r1;
    Uint8 g1;
    Uint8 b1;
    Uint8 a1;
    Uint8 r2;
    Uint8 g2;
    Uint8 b2;
    Uint8 a2;
    SDL_Point from;
    SDL_Point to;
} Gradient;

typedef enum {
    ColoredLineTexture,
    ColoredRectTexture,
    GradientTexture
} TextureType;

// struct representing all of our texture types
typedef struct {
    union {
        ColoredLine line;
        ColoredRectangle rect;
        Gradient gradient;
    };
    TextureType type;
} Texture;

static Texture **textures;
static int textures_count = 0; // amount of textures
static int textures_size = 0; // size of textures array

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
    texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

    if (texture == NULL)
        return 1;

    return 0;
}

int draw_update()
{
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    for (int i = 0; i < textures_count; ++i)
    {
        if (i >= textures_size)
            break;

        Texture *texture = textures[i];
        
        if (texture == NULL)
            continue;

        switch (texture->type)
        {
            case ColoredRectTexture:
                ;
                ColoredRectangle rect = texture->rect;
                SDL_SetRenderDrawColor(renderer, rect.r, rect.g, rect.b, rect.a);
                SDL_RenderDrawRect(renderer, &rect.rect);
                SDL_RenderFillRect(renderer, &rect.rect);
                break;

            case ColoredLineTexture:
                ;
                ColoredLine line = texture->line;
                SDL_SetRenderDrawColor(renderer, line.r, line.g, line.b, line.a);
                SDL_RenderDrawLine(renderer, line.from.x, line.from.y, line.to.x, line.to.y);
                break;

            default:
                break;
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    return 0;
}

void get_screen_dimensions(int *w, int *h)
{
    SDL_GetWindowSize(window, w, h);
}

int _realloc_textures(int size);
int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Texture *texture = malloc(sizeof(*texture));

    if (texture == NULL)
        return 1;

    texture->type = ColoredRectTexture;
    texture->rect = (ColoredRectangle) {
        r, g, b, a,
        { x, y, w, h }
    };

    // add to array
    if (textures_count >= textures_size)
        _realloc_textures(textures_size + 100);
    textures[textures_count++] = texture;

    return 0;
}

int draw_line(int x1, int y1, int x2, int y2,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Texture *texture = malloc(sizeof(*texture));

    if (texture == NULL)
        return 1;

    texture->type = ColoredLineTexture;
    texture->line = (ColoredLine) {
        r, g, b, a,
        { x1, y1 },
        { x2, y2 }
    };

    // add to array
    if (textures_count >= textures_size)
        _realloc_textures(textures_size + 100);
    textures[textures_count++] = texture;

    return 0;
}

int draw_gradient_line(int x1, int y1, int x2, int y2,
        Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
        Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2)
{
    Texture *texture = malloc(sizeof(*texture));

    if (texture == NULL)
        return 1;

    texture->type = GradientTexture;
    texture->gradient = (Gradient) {
        r1, g1, b1, a1,
        r2, g2, b2, a2,
        { x1, y1 },
        { x2, y2 }
    };

    // add to array
    if (textures_count >= textures_size)
        _realloc_textures(textures_size + 100);
    textures[textures_count++] = texture;

    return 0;
}

int clear()
{
    for (int i = 0; i < textures_count; ++i)
        free(textures[i]);

    textures_count = 0;

    return 0;
}

/** private stuff **/

int _realloc_textures(int size)
{
    Texture **tmp;
    tmp = realloc(textures, sizeof(*tmp) * size);

    if (tmp == NULL) {
        return 1;
    }

    textures_size = size;
    textures = tmp;

    return 0;
}
