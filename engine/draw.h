#ifndef ENGINE_DRAW_H
#define ENGINE_DRAW_H

#include "SDL.h"
#include "SDL_image.h"

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

// get texture for layer
SDL_Texture* get_texture(int layer);

// load texture image
SDL_Texture* load_texture(const char *filename);

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

// for color manipulation functions
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

// get RGBA color for given raw pixel data for pixel format
Color get_color(Uint32 pixel, const SDL_PixelFormat *format);

// map the RGBA color to a raw pixel for updating surface
Uint32 map_color(Color color, const SDL_PixelFormat *format);

// fill in array of RGBA color values for surface
// NOTE: colors array should be allocated with size of surface->w * surface->h
int get_colors(Color *colors, const SDL_Surface *surface);

// update RGBA color values for surface
// NOTE: colors array should be allocated with size of surface->w * surface->h
// FIXME likely only works with surfaces with alpha component & 8 bits wide
int update_colors(Color *colors, const SDL_Surface *surface);

#endif
