#ifndef ENGINE_RAYCAST_H
#define ENGINE_RAYCAST_H

#include "map.h"

// initialize raycast renderer
int init_raycast();

// destroy raycast renderer
int destroy_raycast();

// TODO
/* typedef struct { */
/*     int distance; // proportional distance to object */
/*     const Tile *tile; // map tile */
/*     int xOffset; // x-offset *within* the tile that the ray hit */
/*     int yOffset; // y-offset *within* the tile that the ray hit */
/* } RaycastHit; */

// do the raycast for the column & return proportional distance to wall
Vector raycast(const Map *map, int x);

#endif
