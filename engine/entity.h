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
    int health;
} Entity;

// represents a sprite that is relative to the player location, and ready for
// rendering to the screen
typedef struct {
    Entity *entity;
    Vector screenPos; // position on the screen
    int height; // height on screen
    int side; // side of screen
    double dist; // perpendicular distance from player
} Sprite;

#endif
