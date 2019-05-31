#ifndef GAME_H
#define GAME_H

#include "stdinc.h"

typedef struct {
    Vector pos; // player position
    double dir; // angle player is facing (in radians)
    double fov; // field of view (in radians)
} Player;

// initialize game data
int init_game();

// free game data
int destroy_game();

// handle input TODO clarify input data types
int handle_input();

// draw to screen
int update();

// get the player struct
Player get_player();

#endif
