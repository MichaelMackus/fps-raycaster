#ifndef ENGINE_RAYCAST_H
#define ENGINE_RAYCAST_H

#include "map.h"

// initialize raycast renderer
int init_raycast();

// destroy raycast renderer
int destroy_raycast();

typedef enum {
    WALL_NORTH,
    WALL_EAST,
    WALL_WEST,
    WALL_SOUTH
} WallSide;

typedef struct {
    Vector rayPos;
    Vector tilePos;
    double distance; // proportional distance to object
    double xOffset; // x-offset *within* the tile that the ray hit
    double yOffset; // y-offset *within* the tile that the ray hit
    WallSide side; // which side of wall was hit? (useful for simplistic lighting)
} Ray;

// do the raycast for the column & return proportional distance to wall
Ray raycast(const Map *map, int x);

typedef struct {
    Vector tilePos;
    double distance; // proportional distance to object
    double xOffset; // x-offset *within* the tile that the ray hit
    double yOffset; // y-offset *within* the tile that the ray hit
} FloorRay;

// do a floorcast for the first column of the row
FloorRay floorcast(const Map *map, int y);

#endif
