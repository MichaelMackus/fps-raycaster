#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include "SDL.h"

// represents a texture in a texture atlas
typedef struct {
    int width;
    int height;
    int xOffset;
    int yOffset;
    SDL_Texture *atlas;
} SubTexture;

// TODO texture loading

#endif
