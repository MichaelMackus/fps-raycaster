#ifndef DRAW_H
#define DRAW_H

#include "SDL.h"

// initialize drawing
int draw_init(SDL_Renderer *renderer);

// draw to the screen
int draw_update();

// draw a rectangle
int draw_rect(int x, int y, int w, int h,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#endif
