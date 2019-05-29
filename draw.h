#ifndef DRAW_H
#define DRAW_H

#include "SDL.h"
#include "SDL_image.h"

// see: https://wiki.libsdl.org/SDL_GetKeyName
// also: https://discourse.libsdl.org/t/convert-sdl2-scancode-to-ascii-character/23074
typedef struct {
    const char *key;
} Input;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Pixel;

typedef struct {
    int width;
    int height;
    const Pixel *pixels;
    void *data; // internal representation of texture (TODO perhaps use internal list for this)
} Texture;

// initialize drawing
int draw_init(SDL_Window *window, SDL_Renderer *renderer);

// free memory (cannot call other draw calls without draw_init again)
int draw_free();

// get renderer
SDL_Renderer* get_renderer();

// initialize layer - this should be called once per draw loop per layer
int draw_init_layer(int layer, int colorMode, int accessMode, int alphaBlend);

// set the layer for drawing
//
// This will implicitly call draw_init_layer for the layer. Do not rely on
// this when calling draw_start with out of order layers.
int draw_start(int layer);

// draw to the screen
int draw_update(int layer);


// need draw code for:
//
//  1) drawing sprites/textures on the screen - particularly, slices of the textures
//  2) copying textures in different positions & orientations (i.e. flipped)
//  3) updating pixels directly within textures (e.g. with SDL_TEXTUREACCESS_STREAMING)

// updates pixels in texture with passed pixels array
int update_pixels(Texture *texture, const Pixel *pixels);

//
// need load code for:
//
//  1) getting a texture with filled rgba values from a file, irregardless of format
Texture* load_texture(const char *filename);
//  2) getting multiple pieces of a texture (i.e. "sprites")
//  3) getting a pointer to the texture's pixels to modify (e.g. with SDL_TEXTUREACCESS_STREAMING)


// get texture for layer
SDL_Texture* get_texture(int layer);

// draw portion of texture
int draw_texture(SDL_Texture *texture,
        int x1, int y1, int w1, int h1,
        int x2, int y2, int w2, int h2);

// get width & height of screen
void get_screen_dimensions(int *w, int *h);

// draw a rectangle
int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a);

// draw a line
int draw_line(int x1, int y1, int x2, int y2,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a);

// draw a gradient
int draw_gradient(int x, int y, int w, int h, int steps,
        Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
        Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2);

#endif
