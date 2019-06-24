#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include "draw.h"

// TODO should this *contain* subtextures?
typedef struct {
    int width;
    int height;
    Color *pixels;
    SDL_Texture *texture;
} TextureAtlas;

// represents a texture in a texture atlas
typedef struct {
    int width;
    int height;
    int xOffset;
    int yOffset;
    Color *pixels;
    TextureAtlas *atlas;
} SubTexture;

TextureAtlas* create_atlas(const char *fileName);
void free_atlas(TextureAtlas *atlas);

SubTexture* create_subtexture(TextureAtlas *atlas,
        int width, int height, int xOffset, int yOffset);
void free_subtexture(SubTexture *texture);

#endif
