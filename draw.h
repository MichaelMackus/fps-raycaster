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

// represents data in pixel format (i.e. an image)
typedef struct {
    int width;
    int height;
    Pixel *pixels;
    void *data; // internal representation of texture (TODO perhaps use internal list for this)
} Texture;

/* // represents slice of a texture (i.e. sprite in a sprite sheet) */ // TODO is this useful?
/* typedef struct { */
/*     const int width; */
/*     const int height; */
/*     const int textureX;     // xpos within texture */
/*     const int textureY;     // ypos within texture */
/*     const Texture *texture; // pointer to parent texture */
/* } TextureSlice; */

// initialize drawing
int draw_init(SDL_Window *window, SDL_Renderer *renderer);

// free memory (cannot call other draw calls without draw_init again)
int draw_free();

// get renderer
SDL_Renderer* get_renderer();

// initialize layer - this should be called once per draw loop per layer
int draw_init_layer(int index, int colorMode, int accessMode, int alphaBlend);

// set the layer for drawing
//
// This will implicitly call draw_init_layer for the layer. Do not rely on
// this when calling draw_start with out of order layers.
int draw_start(int index);

// draw to the screen
int draw_update();

// updates pixels in texture
int update_pixels(Texture *texture);

// load texture from file
Texture* load_texture(const char *filename);

// divide Texture into TextureSlice
/* TextureSlice slice_texture(const Texture *texture, int width, int height, int xpos, int ypos); */

// get texture from layer for drawing
// NOTE: the pixels of the returned texture are write only, and should be updated with update_pixels
Texture* get_layer(int index);

// draw portion of texture into renderer layer
int draw_texture(Texture *texture,
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

// need code for (performance):
//
//  3) getting a pointer to the internal texture's pixels to modify (e.g. with SDL_TEXTUREACCESS_STREAMING)
//      *NOTE* perhaps we could use API similar to stdio? For example (pseudocode):
//              
//          Texture t = load_texture("asdf.png");
//
//          TEXTURE_STREAM s = get_handle(t);
//          for (x = 0 ; x < w )
//              for (y = 0; y < h )
//                  Pixel p;
//                  p.r = x % 255;
//                  p.g = x % 255;
//                  p.b = x % 255;
//                  write_texture(s, p);
//          close_handle(t);

#endif
