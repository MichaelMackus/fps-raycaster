#include "entity.h"

#include <stdio.h>

static Player player;
Player* get_player()
{
    return &player;
}

// sprites
Sprite enemies[ENEMY_COUNT] = { 
    { 6*13 + 5, 0, 0, 0, { 3, 3 } },
    { 13 + 2, 0, 0, 0, { 8, 16 } },
    { 11, 0, 0, 0, { 6, 17 } },
    { 13 + 2, 0, 0, 0, { 6, 6 } },
    { 13 + 2, 0, 0, 0, { 9, 2 } }
};
Sprite* get_enemies()
{
    return enemies;
}

