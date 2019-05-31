#include "draw.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

// textures array initialized by layers
static Texture **layers;
static int layers_size = 0;

// highest texture layer initialized
static int layer_initialized = -1;

// current layer being drawn to
static Texture *layer;
static SDL_Texture *texture;

// internal texture data struct
typedef struct {
    SDL_Texture *sdlTexture;
    SDL_Surface *surface;
    SDL_Surface *converted;
    Texture *parentTexture; // for sprites
} TextureData;

int draw_init(SDL_Window *win, SDL_Renderer *r)
{
    if (r == NULL)
        return 1;

    renderer = r;
    window = win;

    return 0;
}

int draw_free()
{
    for (int i = 0; i < layers_size; ++i)
    {
        Texture *l = layers[i];
        if (l == NULL) continue;

        TextureData *d = l->data;
        if (d != NULL)
        {
            if (d->surface != NULL)
                SDL_FreeSurface(d->surface);
            if (d->converted != NULL)
                SDL_FreeSurface(d->converted);
            if (d->sdlTexture != NULL)
                SDL_DestroyTexture(d->sdlTexture);
            free(d);
        }

        if (l->pixels != NULL)
            free(l->pixels);

        free(l);
    }

    if (layers != NULL) free(layers);

    return 0;
}

SDL_Renderer* get_renderer()
{
    return renderer;
}

int draw_init_layer(int index, int colorMode, int accessMode, int alphaBlend)
{
    if (index >= layers_size)
    {
        // realloc layers
        int new_size = index + 1;
        Texture **tmp = realloc(layers, sizeof(*layers) * new_size);

        if (tmp == NULL)
            return 1;

        // ensure newly allocated memory is initialized to NULL
        memset(tmp + layers_size, 0, sizeof(*layers) * (new_size - layers_size));

        layers = tmp;
        layers_size = new_size;
    }

    if (layers[index] == NULL)
    {
        Texture *l = malloc(sizeof(Texture));
        layers[index] = l;

        get_screen_dimensions(&(l->width), &(l->height));

        // initialize texture, or error
        SDL_Texture *t = SDL_CreateTexture(renderer,
                colorMode, accessMode, l->width, l->height);

        l->pixels = NULL;

        TextureData *data = malloc(sizeof(TextureData));
        l->data = data;

        data->sdlTexture = t;
        data->surface = NULL;
        data->converted = NULL;
        data->parentTexture = NULL;
    }

    layer = layers[index];
    TextureData *d = layer->data;
    texture = d->sdlTexture;
    SDL_SetRenderTarget(renderer, texture);

    if (alphaBlend)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    layer_initialized = index;

    return 0;
}

int draw_start(int index)
{
    // only initialize layer first time
    if (layer_initialized < index) draw_init_layer(index, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1);

    layer = layers[index];
    TextureData *d = layer->data;
    texture = d->sdlTexture;
    SDL_SetRenderTarget(renderer, texture);

    return 0;
}

int draw_update()
{
    SDL_SetRenderTarget(renderer, NULL);

    // copy layers in order into renderer
    if (layers_size > 0)
    {
        for (int i = 0; i < layers_size; i++)
        {
            Texture *curLayer = layers[i];

            if (curLayer == NULL)
                continue;

            TextureData *d = curLayer->data;
            SDL_RenderCopy(renderer, d->sdlTexture, NULL, NULL);
        }
    }

    // reset initialized layer
    layer_initialized = - 1;

    SDL_RenderPresent(renderer);

    return 0;
}

Texture* get_layer(int index)
{
    Texture *l = layers[index];

    if (l->pixels == NULL)
        l->pixels = malloc(sizeof(Pixel) * l->width * l->height);

    return l;
}

int update_pixels(Texture *texture)
{
    Pixel *pixels = texture->pixels;
    TextureData *data = texture->data;

    // create temporary pixels container
    Uint32 *tmp = malloc(sizeof(*tmp) * texture->width * texture->height);
    if (tmp == NULL) return 1;

    for (int y = 0; y < texture->height; ++y)
    {
        for (int x = 0; x < texture->width; ++x)
        {
            const unsigned int offset = (data->surface->pitch/4)*y + x;

            // it is important we do not use the format of the surface, since that could be different
            tmp[offset] = SDL_MapRGBA(data->converted->format,
                    pixels[offset].r,
                    pixels[offset].g,
                    pixels[offset].b,
                    pixels[offset].a);
        }
    }

    if (SDL_UpdateTexture(data->sdlTexture,
            NULL,
            tmp,
            data->surface->pitch) != 0)
        return 1;

    free(tmp);

    return 0;
}

Texture* load_texture(const char *filename)
{
    SDL_Surface *surface = IMG_Load(filename);

    if (surface == NULL)
        return NULL;

    Texture *texture;
    texture = malloc(sizeof(*texture));
    // TODO need free_texture call

    if (texture == NULL)
        return NULL;

    texture->width = surface->w;
    texture->height = surface->h;

    SDL_Texture *sdlTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(sdlTexture, SDL_BLENDMODE_BLEND); // TODO configure alpha somewhere

    TextureData *data = malloc(sizeof(TextureData));
    texture->data = data;

    data->sdlTexture = sdlTexture;
    data->surface = surface;
    data->parentTexture = NULL;

    // update texture data with format of texture
    Uint32 format;
    SDL_QueryTexture(sdlTexture, &format, NULL, NULL, NULL);

    // store converted surface format for pixel updates
    data->converted = SDL_ConvertSurfaceFormat(surface, format, 0);

    // fill with RGB values
    {
        Uint8 *rawPixels = surface->pixels;
        Pixel *targetPixels = malloc(sizeof(Pixel) * surface->w * surface->h);
        if (targetPixels == NULL) return NULL;
        texture->pixels = targetPixels;

        for (int y = 0; y < surface->h; ++y)
        {
            for (int x = 0; x < surface->w; ++x)
            {
                // Get the single pixel. We use memcpy here in order to
                // account for non-32bit color spaces. For example, if an image
                // doesn't have an alpha channel, color values might be 24
                // bits, so we need to copy 3 bytes (BytesPerPixel) into the 4
                // bytes of (p of type Uint32) on the stack.
                //
                // NOTE: need to test this works for non-8bit formats
                Uint32 p;
                memcpy(&p, rawPixels, surface->format->BytesPerPixel);

                /* const unsigned int offset = (surface->pitch/4)*y + x; // pitch is in bytes (32/4=8 bits per byte) */
                Uint8 r,g,b;
                SDL_GetRGBA(p,
                        surface->format,
                        &(targetPixels->r),
                        &(targetPixels->g),
                        &(targetPixels->b),
                        &(targetPixels->a));

                rawPixels += surface->format->BytesPerPixel;
                targetPixels ++;
            }
        }
    }


    printf("File: %s\n",
            filename);
    printf("Surface: %s\n",
            SDL_GetPixelFormatName(surface->format->format));
    printf("Converted Surface: %s\n",
            SDL_GetPixelFormatName(data->converted->format->format));
    printf("Texture: %s\n",
            SDL_GetPixelFormatName(format));
    printf("Pitch: %d Converted Pitch: %d\n",
            data->surface->pitch, data->converted->pitch);

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
