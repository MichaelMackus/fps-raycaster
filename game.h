#ifndef GAME_H
#define GAME_H

#define MAP_WIDTH 19
#define MAP_HEIGHT 19
#define MAP_MAX_DISTANCE MAP_WIDTH*MAP_WIDTH + MAP_HEIGHT*MAP_HEIGHT
#define ENEMY_COUNT 5

#include "stdinc.h"

// initialize game data
int init_game();

typedef struct {
    Vector pos; // player position
    double dir; // angle player is facing (in radians)
    double fov; // field of view (in radians)
} Player;

Player* get_player();
const char* get_map();
const char get_tile(int x, int y);

// representing sprite on screen
typedef struct {
    int index; // index in sprite sheet
    double distX; // perpendicular X distance from player
    double distY; // perpendicular Y distance (depth) from player
    double angle; // angle from player dir
    Vector pos; // position on 2d map
} Sprite;

Sprite* get_enemies();

#endif
