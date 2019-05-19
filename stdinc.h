#ifndef STDINC_H
#define STDINC_H

#include "math.h"

#define PI acos(-1)

typedef struct {
    float x;
    float y;
} Vector;

// convert degrees to radians
float to_radians(float degrees)
{
    float radiansPerDegree = PI / 180;

    return degrees * radiansPerDegree;
}

// convert radians to degrees
float to_degrees(float radians)
{
    float degreesPerRadian = 180 / PI;

    return radians * degreesPerRadian;
}

// calculate distance between vectors
float distance(Vector from, Vector to)
{
    return sqrt(pow(to.x - from.x, 2) +
            pow(to.y - from.y, 2));
}

#endif
