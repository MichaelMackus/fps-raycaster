#ifndef STDINC_H
#define STDINC_H

#include "math.h"

#define PI acos(-1)

typedef struct {
    double x;
    double y;
} Vector;

// convert degrees to radians
static inline double to_radians(double degrees)
{
    double radiansPerDegree = PI / 180;

    return degrees * radiansPerDegree;
}

// convert radians to degrees
static inline double to_degrees(double radians)
{
    double degreesPerRadian = 180 / PI;
    double degrees = radians * degreesPerRadian;

    return degrees;
}

// calculate distance between vectors
static inline double distance(Vector from, Vector to)
{
    return sqrt(pow(to.x - from.x, 2) +
            pow(to.y - from.y, 2));
}

// what quadrant is an angle
// returns 1-4 (quadrant)
static inline int quadrant(int degrees)
{
    int degrees360 = (int) degrees % 360;

    // flip degrees when negative
    if (degrees360 < 1) {
        degrees360 = 360 + degrees360;
    }

    if (degrees360 <= 90) {
        return 1;
    } else if (degrees360 <= 180) {
        return 2;
    } else if (degrees360 <= 270) {
        return 3;
    } else if (degrees360 <= 360) {
        return 4;
    }

    // error
    return 0;
}

#endif
