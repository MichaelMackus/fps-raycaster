#ifndef STDINC_H
#define STDINC_H

#include "math.h"

#define PI acos(-1)

typedef struct {
    double x;
    double y;
} Vector;

// rotate an angle by amount, ensuring it stays >= 0 and < 2*PI
static inline double rotate(double angle, double amount)
{
    double newAngle = angle + amount;

    while (newAngle < 0)
    {
        newAngle = 2*PI + newAngle;
    }

    while (newAngle >= 2*PI)
    {
        newAngle -= 2*PI;
    }

    return newAngle;
}

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

// returns 1-4 (quadrant)
static inline int quadrant(double radians)
{
    // corrected radians
    radians = rotate(0.0f, radians);

    if (radians < PI / 2.0f)
        return 1;
    else if (radians < PI)
        return 2;
    else if (radians < 3.0f * PI / 2.0f)
        return 3;
    else if (radians < 2.0f * PI)
        return 4;

    // error
    return 0;
}

#endif
