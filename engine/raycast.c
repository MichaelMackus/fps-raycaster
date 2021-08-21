#include "raycast.h"
#include "entity.h"
#include "map.h"
#include "stdinc.h"

#include "SDL.h"

int screenWidth;
int screenHeight;
double distanceToSurface = 0;
Player *player = NULL;

int init_raycast(int w, int h)
{
    screenWidth = w;
    screenHeight = h;

    return 0;
}

int destroy_raycast()
{
    return 0;
}

// whether vector hits wall side
int hit_wall(const Map *map, Vector pos, WallSide side)
{
    if (side == WALL_SOUTH)
    {
        pos.y -= 1;
    }

    if (side == WALL_EAST)
    {
        pos.x -= 1;
    }

    return is_passable(map, pos.x, pos.y) == 0;
}

Ray raycast(const Map *map, int x)
{
    Player *player = get_player();

    // calculate distance from player to screen - this will be screenWidth/2 for 90 degree FOV
    if (distanceToSurface == 0)
        distanceToSurface = (screenWidth/2.0) / tan(player->fov/2);

    // calculate ray direction
    /* double rayDir = player->dir - (player->fov/2) + x * (player->fov/screenWidth); // generates distortion towards edges */
    // fix for increased distortion towards screen edges
    // see: https://stackoverflow.com/questions/24173966/raycasting-engine-rendering-creating-slight-distortion-increasing-towards-edges
    double rayDir = player->dir + atan((x - screenWidth/2.0) / distanceToSurface);

    // set tileStepX and tileStepY
    int tileStepX = 0;
    int tileStepY = 0;
    int q = quadrant(rayDir);
    if (q == 1)
    {
        tileStepX = 1;
        tileStepY = 1;
    }
    else if (q == 2)
    {
        tileStepX = -1;
        tileStepY = 1;
    }
    else if (q == 3)
    {
        tileStepX = -1;
        tileStepY = -1;
    }
    else if (q == 4)
    {
        tileStepX = 1;
        tileStepY = -1;
    }

    // calculate length until next cell is reached in x & y
    Vector cellDistance;
    if (tileStepX == 1)
        cellDistance.x = 1 - (player->pos.x - floor(player->pos.x));
    else
        cellDistance.x = player->pos.x - floor(player->pos.x);
    if (tileStepY == 1)
        cellDistance.y = 1 - (player->pos.y - floor(player->pos.y));
    else
        cellDistance.y = player->pos.y - floor(player->pos.y);
    // edge case to handle 0 distance TODO clean this up
    if (cellDistance.x == 0)
        cellDistance.x = 1;
    if (cellDistance.y == 0)
        cellDistance.y = 1;

    // calculate y & x intercept offset from player
    double yInterceptOffset = cellDistance.x * tan(rayDir);
    if (yInterceptOffset < 0) // TODO cleanup
        yInterceptOffset = yInterceptOffset * -1;
    double xInterceptOffset = cellDistance.y / tan(rayDir);
    if (xInterceptOffset < 0) // TODO cleanup
        xInterceptOffset = xInterceptOffset * -1;

    // setup rays for detecting cell walls
    Vector yIntercept;
    yIntercept.x = player->pos.x + (cellDistance.x * (double) tileStepX);
    yIntercept.y = player->pos.y + (yInterceptOffset * (double) tileStepY);
    Vector xIntercept;
    xIntercept.x = player->pos.x + (xInterceptOffset * (double) tileStepX);
    xIntercept.y = player->pos.y + (cellDistance.y * (double) tileStepY);

    // calculate ystep and xstep for ray projection
    // TODO using float here as a quick fix for precision issues
    // TODO need to check for both conditions below in hit loop in order to prevent endless loop
    float xstep = rayDir == 0 ? 0 : 1 / tan(rayDir);
    if (xstep < 0) // TODO cleanup
        xstep = xstep * -1;
    xstep = xstep * tileStepX;
    float ystep = tan(rayDir);
    if (ystep < 0) // TODO cleanup
        ystep = ystep * -1;
    ystep = ystep * tileStepY;

#ifdef GAME_DEBUG
    printf("quadrant: %d, degrees: %f, dir: %f, tan dir: %f, celld: (%f, %f), intercepts: (%f, %f), player: (%f, %f), x-intercept: (%f, %f), y-intercept: (%f, %f)\n",
            q, to_degrees(rayDir), rayDir, 
            tan(rayDir),
            cellDistance.x, cellDistance.y,
            xInterceptOffset, yInterceptOffset,
            player->pos.x, player->pos.y,
            xIntercept.x, xIntercept.y, yIntercept.x, yIntercept.y);
#endif

    // increment ray pos until we hit wall, *or* go past map bounds
    int hit = 0;
    int i = 0;
    WallSide side; // 0 for x-intercept, 1 for y-intercept
    while (hit == 0)
    {
        // hack for bounds check TODO fix this
        if (++i > 1000) {
            printf("Raycast error, angle: %f, rayDir: %f\n", player->dir, rayDir);
            break;
        }
        // check for x-intercept
        if (tileStepY == 1)
        {
            while (xIntercept.y <= yIntercept.y && hit == 0)
            {
                if (hit_wall(map, xIntercept, WALL_NORTH))
                {
                    hit = 1;
                    side = WALL_NORTH;
                    break;
                }
                xIntercept.x += xstep;
                xIntercept.y += tileStepY;
            }
        }
        else
        {
            while (xIntercept.y >= yIntercept.y && hit == 0)
            {
                if (hit_wall(map, xIntercept, WALL_SOUTH))
                {
                    hit = 1;
                    side = WALL_SOUTH;
                    break;
                }
                xIntercept.x += xstep;
                xIntercept.y += tileStepY;
            }
        }

        // check for y-intercept
        if (tileStepX == 1)
        {
            while (yIntercept.x <= xIntercept.x && hit == 0)
            {
                if (hit_wall(map, yIntercept, WALL_WEST))
                {
                    hit = 1;
                    side = WALL_WEST;
                    break;
                }
                yIntercept.y += ystep;
                yIntercept.x += tileStepX;
            }
        }
        else
        {
            while (yIntercept.x >= xIntercept.x && hit == 0)
            {
                if (hit_wall(map, yIntercept, WALL_EAST))
                {
                    hit = 1;
                    side = WALL_EAST;
                    break;
                }
                yIntercept.y += ystep;
                yIntercept.x += tileStepX;
            }
        }
    }

    if (hit)
    {
        Vector rayPos;
        if (side == WALL_NORTH || side == WALL_SOUTH)
        {
            rayPos.x = xIntercept.x;
            rayPos.y = xIntercept.y;
        }
        else
        {
            rayPos.x = yIntercept.x;
            rayPos.y = yIntercept.y;
        }

#ifdef GAME_DEBUG
        printf("hit found: (%f, %f); side: %d\n", rayPos.x, rayPos.y, side);
#endif

        // calculate proportional distance (corrects for fisheye effect)
        /* double propDist = distance(rayPos, player->pos) * cos(rayDir - player->dir); */
        // more efficient way:
        // delta x = d * cos(rayDir.x), delta y = d * cos(rayDir.y)
        // which expands into:
        double propDist = cos(player->dir) * (rayPos.x - player->pos.x) + sin(player->dir) * (rayPos.y - player->pos.y);

        Vector tilePos = rayPos;

        // ensure we're not drawing the starting wall tile TODO remove this & cleanup rayPos
        /* if ((int) tilePos.x == (int) rayPos.x && */
        /*     (int) tilePos.y == (int) rayPos.y) */
        /* { */
        /*     if (side == WALL_NORTH) */
        /*         tilePos.y -= 1; */
        /*     if (side == WALL_WEST) */
        /*         tilePos.x -= 1; */
        /* } */

        if (side == WALL_SOUTH)
            tilePos.y -= 1;
        if (side == WALL_EAST)
            tilePos.x -= 1;

        // calculate which part of texture to render
        /* Vector difference = (Vector) { rayPos.x - floor(rayPos.x), rayPos.y - floor(rayPos.y) }; */ // using vector subtraction
        /* double wallX = sqrt(difference.x*difference.x + difference.y*difference.y); */
        // more efficient:
        double wallX = rayPos.x - floor(rayPos.x); // where within the wall did the ray hit
        if (side == WALL_NORTH || side == WALL_SOUTH) wallX = rayPos.x - floor(rayPos.x);
        else wallX = rayPos.y - floor(rayPos.y);

        Ray ray;
        ray.rayPos = rayPos;
        ray.tilePos = tilePos;
        ray.distance = propDist;
        ray.xOffset = wallX;
        ray.side = side;

        return ray;
    }

    Ray ray;
    ray.rayPos = player->pos;
    ray.tilePos = player->pos;
    ray.distance = 9999;

    return ray;
}

Vector get_near_plane_left()
{
    double rayDist = 1/sin(player->fov/2);
    double dir = rotate(player->dir, -1 * player->fov/2);
    Vector vec;
    vec.x = rayDist * cos(dir);
    vec.y = rayDist * sin(dir);

    return vec;
}
Vector get_near_plane_right()
{
    double rayDist = 1/sin(player->fov/2);
    double dir = rotate(player->dir, player->fov/2);
    Vector vec;
    vec.x = rayDist * cos(dir);
    vec.y = rayDist * sin(dir);

    return vec;
}
Vector get_far_plane_left(double dist)
{
    double dir = rotate(player->dir, -1 * player->fov/2);
    Vector vec;
    vec.x = dist * cos(dir);
    vec.y = dist * sin(dir);

    return vec;
}
Vector get_far_plane_right(double dist)
{
    double dir = rotate(player->dir, player->fov/2);
    Vector vec;
    vec.x = dist * cos(dir);
    vec.y = dist * sin(dir);

    return vec;
}

// do a floorcast for the row & return ray to first column of floor
FloorRay floorcast(const Map *map, int y)
{
    // TODO ensure this is always initialized
    if (player == NULL)
        player = get_player();

    // calculate distance from player to screen - this will be screenWidth/2 for 90 degree FOV
    if (distanceToSurface == 0)
        distanceToSurface = (screenWidth/2.0) / tan(player->fov/2);

    FloorRay ray;

    // first pass, initialize variables
    double currentDist = screenHeight / (screenHeight - 2.0 * y);
    Vector fovLeft = get_near_plane_left();
    Vector tilePos;
    tilePos.x = player->pos.x + currentDist * fovLeft.x;
    tilePos.y = player->pos.y + currentDist * fovLeft.y;
    ray.tilePos = tilePos;

    Vector interval;

    // first pass, initialize variables
    Vector ray1 = get_near_plane_left();
    Vector ray2 = get_near_plane_right();

    ray.distance = currentDist;
    ray.xOffset = currentDist * (ray2.x - ray1.x) / (distanceToSurface*2);
    ray.yOffset = currentDist * (ray2.y - ray1.y) / (distanceToSurface*2);

    return ray;
}
