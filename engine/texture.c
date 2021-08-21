#include "texture.h"

int get_colors(Color *colors, const SDL_Surface *surface)
{
    Uint8 *rawPixels = surface->pixels;
    Color *targetColor = colors;

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

            SDL_GetRGBA(p,
                    surface->format,
                    &(targetColor->r),
                    &(targetColor->g),
                    &(targetColor->b),
                    &(targetColor->a));

            rawPixels += surface->format->BytesPerPixel;
            targetColor ++;
        }
    }

    return 0;
}

TextureAtlas* create_atlas(const char *fileName)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    Color *colors;
    TextureAtlas *atlas;

    // load SDL texture & surface
    {
        // load our spritesTexture image
        surface = IMG_Load(fileName);

        if (surface == NULL)
            return NULL;

        /* texture = SDL_CreateTextureFromSurface(get_renderer(), surface); */

        /* if (texture == NULL) */
        /* { */
        /*     SDL_FreeSurface(surface); */

        /*     return NULL; */
        /* } */

        // TODO is this necessary?
        /* // ensure format is RGBA with 32-bits for color manipulation */
        /* SDL_Surface *tmp = SDL_ConvertSurfaceFormat(textureSurface, SDL_PIXELFORMAT_ARGB8888, 0); */
        /* if (tmp == NULL) return 1; */
        /* SDL_FreeSurface(textureSurface); */
        /* textureSurface = tmp; */

        // turn on alpha blending TODO probably want a flag for this
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    // load colors
    {
        colors = malloc(sizeof(*colors) * surface->w * surface->h);

        if (colors == NULL)
        {
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);

            return NULL;
        }

        if (get_colors(colors, surface) != 0)
        {
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
            free(colors);

            return NULL;
        }
    }

    // create atlas
    {
        atlas = malloc(sizeof(*atlas));

        if (atlas == NULL)
        {
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
            free(colors);

            return NULL;
        }

        atlas->width = surface->w;
        atlas->height = surface->h;
        atlas->pixels = colors;
        atlas->texture = texture;
        atlas->surface = surface;
        atlas->subtextures = NULL;
        atlas->subtextureAmount = 0;
    }

    return atlas;
}
void free_atlas(TextureAtlas *atlas);

int insert_subtexture(TextureAtlas *atlas, SubTexture *subtexture)
{
    SubTexture **tmp = realloc(atlas->subtextures, sizeof(*tmp) * ++atlas->subtextureAmount);

    if (tmp == NULL)
    {
#ifdef GAME_DEBUG
        printf("Error reallocating subtextures!\n");
#endif

        return -1;
    }

    atlas->subtextures = tmp;
    atlas->subtextures[atlas->subtextureAmount - 1] = subtexture;

    return atlas->subtextureAmount - 1;
}

SubTexture* create_subtexture(TextureAtlas *atlas, int width, int height, int xOffset, int yOffset)
{
    if (atlas == NULL)
        return NULL;

    SubTexture *texture = malloc(sizeof(*texture));
    if (texture == NULL)
        return texture;

    if (xOffset + width > atlas->width ||
            yOffset + height > atlas->height)
    {
#ifdef GAME_DEBUG
        printf("Error creating subtexture - location not within TextureAtlas!\n");
#endif

        return NULL;
    }

    texture->width = width;
    texture->height = height;
    texture->xOffset = xOffset;
    texture->yOffset = yOffset;
    texture->atlas = atlas;

    // offset subtexture colors
    // TODO clamp this to *only* the subtexture colors
    // TODO currently, this needs the atlas pitch to lookup a color
    Color *pixels = atlas->pixels;
    pixels += yOffset*width + xOffset;
    texture->pixels = pixels;

    return texture;
}

int populate_atlas(TextureAtlas *atlas, int subtextureWidth, int subtextureHeight)
{
    // simple bounds check
    if (subtextureWidth > atlas->width || subtextureHeight > atlas->height)
    {
#ifdef GAME_DEBUG
        printf("Error populating atlas - subtexture size outside atlas bounds!\n");
#endif

        return 0;
    }

    // free the subtextures array if exists
    if (atlas->subtextureAmount > 0)
        atlas->subtextureAmount = 0;
    if (atlas->subtextures != NULL)
    {
        free(atlas->subtextures);
        atlas->subtextures = NULL;
    }

    int cols = atlas->width / subtextureWidth;
    int rows = atlas->height / subtextureHeight;
    
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            int xOffset = x * subtextureWidth;
            int yOffset = y * subtextureHeight;
            SubTexture *subtexture = create_subtexture(atlas,
                    subtextureWidth, subtextureHeight, xOffset, yOffset);
            insert_subtexture(atlas, subtexture);
        }
    }

    return atlas->subtextureAmount;
}

void free_subtexture(SubTexture *texture);

int update_colors(Color *colors, const SDL_Surface *surface)
{
    // assert the surface format is valid TODO add functionality for other formats
    if (surface->format->BytesPerPixel != 4)
    {
#ifdef GAME_DEBUG
        printf("update_colors error: BytesPerPixel of surface does not equal 4!\n");
#endif
        return 1;
    }

    Uint32 *tmp = surface->pixels;

    for (int y = 0; y < surface->h; ++y)
    {
        for (int x = 0; x < surface->w; ++x)
        {
            // it is important we do not use the format of the surface, since that could be different
            const unsigned int pixelOffset = surface->w*y + x;
            Uint32 p = SDL_MapRGBA(surface->format,
                    colors[pixelOffset].r,
                    colors[pixelOffset].g,
                    colors[pixelOffset].b,
                    colors[pixelOffset].a);

            const unsigned int offset = (surface->pitch/4)*y + x;
            tmp[offset] = p;
        }
    }

    return 0;
}
