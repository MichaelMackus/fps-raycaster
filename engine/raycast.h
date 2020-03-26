#ifndef ENGINE_RAYCAST_H
#define ENGINE_RAYCAST_H

#include "map.h"

// initialize raycast renderer
int init_raycast();

// destroy raycast renderer
int destroy_raycast();

typedef struct {
    Vector rayPos;
    Vector tilePos;
    double distance; // proportional distance to object
    double xOffset; // x-offset *within* the tile that the ray hit
    double yOffset; // y-offset *within* the tile that the ray hit
} Ray;

// do the raycast for the column & return proportional distance to wall
Ray raycast(const Map *map, int x);

// do a floorcast for the first column of the row
Vector floorcast(const Map *map, int y);

// get a vector that represents the interval to add to the floorcast for
// each step of x column (this avoids costly trig during looping)
Vector floorcast_get_interval(const Map *map, int y);

#endif
