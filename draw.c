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

static ColoredRectangle **rectangles; // array of rectangles
static int rect_count = 0; // amount of textures
static int rect_size = 0; // size of textures array

// represent a colored line in SDL
typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_Point from;
    SDL_Point to;
} ColoredLine;

static ColoredLine **lines; // array of lines
static int line_count = 0;
static int line_size = 0;

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

    for (int i = 0; i < rect_count; ++i)
    {
        if (i > rect_size)
            break;

        ColoredRectangle *rect = rectangles[i];
        
        if (rect == NULL)
            continue;

        SDL_SetRenderDrawColor(renderer, rect->r, rect->g, rect->b, rect->a);
        SDL_RenderDrawRect(renderer, &rect->rect);
        SDL_RenderFillRect(renderer, &rect->rect);
    }

    for (int i = 0; i < line_count; ++i)
    {
        if (i > line_size)
            break;

        ColoredLine *line = lines[i];
        
        if (line == NULL)
            continue;

        SDL_SetRenderDrawColor(renderer, line->r, line->g, line->b, line->a);
        SDL_RenderDrawLine(renderer, line->from.x, line->from.y, line->to.x, line->to.y);
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

int _realloc_rects(int size);
int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    ColoredRectangle *coloredRect = malloc(sizeof(*coloredRect));

    if (coloredRect == NULL)
        return 1;

    coloredRect->rect.x = x;
    coloredRect->rect.y = y;
    coloredRect->rect.w = w;
    coloredRect->rect.h = h;

    coloredRect->r = r;
    coloredRect->g = g;
    coloredRect->b = b;
    coloredRect->a = a;

    // add to array
    if (rect_count >= rect_size)
        _realloc_rects(rect_size + 100);
    rectangles[rect_count++] = coloredRect;

    return 0;
}

int _realloc_lines(int size);
int draw_line(int x1, int y1, int x2, int y2,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    ColoredLine *line = malloc(sizeof(*line));

    line->r = r;
    line->g = g;
    line->b = b;
    line->a = a;

    /* line->from = (SDL_Point) { x1, y1 }; */
    /* line->to = (SDL_Point) { x2, y2 }; */
    line->from.x = x1;
    line->from.y = y1;
    line->to.x = x2;
    line->to.y = y2;

    // add to array
    if (line_count >= line_size)
        _realloc_lines(line_size + 100);
    lines[line_count++] = line;

    return 0;
}

int clear()
{
    // for now, just reset count
    rect_count = 0;
    line_count = 0;

    return 0;
}

/** private stuff **/

int _realloc_rects(int size)
{
    ColoredRectangle **tmp;
    tmp = realloc(rectangles, sizeof(*tmp) * size);

    if (tmp == NULL) {
        return 1;
    }

    rect_size = size;
    rectangles = tmp;

    return 0;
}

int _realloc_lines(int size)
{
    ColoredLine **tmp;
    tmp = realloc(lines, sizeof(*tmp) * size);

    if (tmp == NULL) {
        return 1;
    }

    line_size = size;
    lines = tmp;

    return 0;
}
