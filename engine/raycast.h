#ifndef ENGINE_RAYCAST_H
#define ENGINE_RAYCAST_H

#include "map.h"

// initialize raycast renderer
int init_raycast();

// destroy raycast renderer
int destroy_raycast();

// do the raycast & draw to the screen
int raycast(const Map* map);

#endif
