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

Player* get_player();

typedef enum {
    ENTITY_ENEMY,
    ENTITY_PROJECTILE
} EntityType;

// representing sprite on screen
typedef struct {
    SubTexture *texture;
    EntityType type;
    Vector pos; // position on 2d map
    double dir; // heading direction
} Entity;

// represents a sprite that is relative to the player location, and ready for
// rendering to the screen
typedef struct {
    Entity *entity;
    double distX; // perpendicular X distance from player
    double distY; // perpendicular Y distance (depth) from player
    double angle; // angle from player dir
    int side; // side of screen
} Sprite;

#endif
