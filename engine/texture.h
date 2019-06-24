#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include "draw.h"

typedef struct {
    int width;
    int height;
    Color *pixels;
    SDL_Texture *texture;
    int subtextureAmount;
    struct SubTexture **subtextures;
} TextureAtlas;

// represents a texture in a texture atlas
typedef struct SubTexture {
    int width;
    int height;
    int xOffset;
    int yOffset;
    Color *pixels;
    TextureAtlas *atlas;
} SubTexture;

TextureAtlas* create_atlas(const char *fileName);
void free_atlas(TextureAtlas *atlas);

// populate the atlas subtextures (assumes a constant subtexture size)
// returns amount of subtextures
int populate_atlas(TextureAtlas *atlas, int subtextureWidth, int subtextureHeight);

// create subtexture
SubTexture* create_subtexture(TextureAtlas *atlas,
        int width, int height, int xOffset, int yOffset);
void free_subtexture(SubTexture *texture);

// insert subtexture into the atlas
// returns index of inserted subtexture, or -1 on failure
int insert_subtexture(TextureAtlas *atlas, SubTexture *subtexture);

#endif
