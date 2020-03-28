#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "texture.h"
#include "stdinc.h"

typedef struct {
    Vector pos; // player position
    double dir; // angle player is facing (in radians)
    double fov; // field of view (in radians)
    int shooting; // whether player is trying to shoot
} Player;

typedef enum {
    SPRITE_ENEMY,
    SPRITE_PROJECTILE
} SpriteType;

// representing sprite on screen
typedef struct {
    SubTexture *texture;
    SpriteType type;
    double distX; // perpendicular X distance from player
    double distY; // perpendicular Y distance (depth) from player
    double angle; // angle from player dir
    Vector pos; // position on 2d map
    double dir; // direction projectile is heading
} Sprite;

Player* get_player();

#endif
