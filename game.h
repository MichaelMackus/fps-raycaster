#ifndef GAME_H
#define GAME_H

// initialize game data
void init_game();

// represents one tick (frame) in the game loop
// return 0 = quit, 1 = continue
int tick_game();

#endif
