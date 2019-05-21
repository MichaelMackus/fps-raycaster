#ifndef GAME_H
#define GAME_H

#include "stdinc.h"

typedef struct {
    Vector pos; // player position
    double dir; // angle player is facing (in radians)
    double fov; // field of view (in radians)
} Player;

// initialize game data
void init_game();

// represents one tick (frame) in the game loop
// return 0 = quit, 1 = continue
int tick_game();

// get the player struct
Player get_player();

#endif
