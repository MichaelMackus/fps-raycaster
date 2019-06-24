#include "texture.h"

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

        texture = SDL_CreateTextureFromSurface(get_renderer(), surface);

        if (texture == NULL)
        {
            SDL_FreeSurface(surface);

            return NULL;
        }

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
    }

    // don't need anymore
    SDL_FreeSurface(surface);

    return atlas;
}
void free_atlas(TextureAtlas *atlas);

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
    Color *pixels = atlas->pixels;
    pixels += yOffset*width + xOffset;
    texture->pixels = pixels;

    return texture;
}
void free_subtexture(SubTexture *texture);
