#ifndef DRAW_H
#define DRAW_H

#include "SDL.h"
#include "SDL_image.h"

// initialize drawing
int draw_init(SDL_Window *window, SDL_Renderer *renderer);

// clear & initialize target layer for drawing
int draw_start(int layer);

// draw to the screen
int draw_update(int layer);

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

#endif
