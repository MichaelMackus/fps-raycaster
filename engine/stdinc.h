#ifndef STDINC_H
#define STDINC_H

#include "math.h"

#define PI acos(-1)

typedef struct {
    double x;
    double y;
} Vector;

// check collision (returns 1 if collision found)
static inline int check_collision(Vector v1, Vector v2, int w1, int w2)
{
    if (((v1.x - w1/2.0 < v2.x - w2/2.0 && v1.x + w1/2.0 > v2.x - w2/2.0) ||
         (v1.x - w1/2.0 < v2.x + w2/2.0 && v1.x + w1/2.0 > v2.x + w2/2.0)) &&
        ((v1.y - w1/2.0 < v2.y - w2/2.0 && v1.y + w1/2.0 > v2.y - w2/2.0) ||
         (v1.y - w1/2.0 < v2.y + w2/2.0 && v1.y + w1/2.0 > v2.y + w2/2.0)))
    {
        return 1;
    }

    return 0;
}

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

static inline double absd(double d)
{
    if (d < 0) return d * -1;
    else return d;
}


// normalize vector to unit length 1
static inline Vector normalize(Vector v)
{
    return (Vector) {
        v.x / sqrt(v.x*v.x + v.y*v.y),
        v.y / sqrt(v.x*v.x + v.y*v.y)
    };
}

// calculate dot product between vectors
static inline double dot_product(Vector v1, Vector v2)
{
    return v1.x*v2.x + v1.y*v2.y;
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
