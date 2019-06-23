#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

// TODO get this from engine
#define ENEMY_COUNT 5

#include "stdinc.h"

typedef struct {
    Vector pos; // player position
    double dir; // angle player is facing (in radians)
    double fov; // field of view (in radians)
} Player;

// representing sprite on screen
typedef struct {
    int index; // index in sprite sheet
    double distX; // perpendicular X distance from player
    double distY; // perpendicular Y distance (depth) from player
    double angle; // angle from player dir
    Vector pos; // position on 2d map
} Sprite;

Player* get_player();
Sprite* get_enemies();

#endif
