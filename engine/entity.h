#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "texture.h"
#include "stdinc.h"

typedef struct {
    Vector pos; // player position
    double dir; // angle player is facing (in radians)
    double fov; // field of view (in radians)
} Player;

// representing sprite on screen
typedef struct {
    SubTexture *texture;
    double distX; // perpendicular X distance from player
    double distY; // perpendicular Y distance (depth) from player
    double angle; // angle from player dir
    Vector pos; // position on 2d map
} Sprite;

Player* get_player();

#endif
