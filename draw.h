#ifndef DRAW_H
#define DRAW_H

#include "SDL.h"

// initialize drawing
int draw_init(SDL_Window *window, SDL_Renderer *renderer);

// draw to the screen
int draw_update();

// get width & height of screen
void get_screen_dimensions(int *w, int *h);

// draw a rectangle
int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a);

// clear rects
int clear_rects();

#endif
