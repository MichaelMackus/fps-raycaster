#ifndef ENGINE_RAYCAST_H
#define ENGINE_RAYCAST_H

#include "map.h"

// initialize raycast renderer
int init_raycast();

// destroy raycast renderer
int destroy_raycast();

// TODO
typedef struct {
    Vector rayPos;
    Vector tilePos;
    double distance; // proportional distance to object
    double xOffset; // x-offset *within* the tile that the ray hit
    double yOffset; // y-offset *within* the tile that the ray hit
} Ray;

// do the raycast for the column & return proportional distance to wall
Ray raycast(const Map *map, int x);

#endif
