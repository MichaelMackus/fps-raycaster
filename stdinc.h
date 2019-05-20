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
    float degrees = radians * degreesPerRadian;

    if (degrees < 0) {
        return 360 + degrees;
    }

    return degrees;
}

// calculate distance between vectors
float distance(Vector from, Vector to)
{
    return sqrt(pow(to.x - from.x, 2) +
            pow(to.y - from.y, 2));
}

// what quadrant is an angle
// returns 1-4 (quadrant)
float quadrant(float degrees)
{
    if (degrees <= 90) {
        return 1;
    } else if (degrees <= 180) {
        return 2;
    } else if (degrees <= 270) {
        return 3;
    } else if (degrees <= 360) {
        return 4;
    }

    // error
    return 0;
}

#endif
