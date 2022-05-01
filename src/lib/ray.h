#ifndef TEAM32_RAY_H
#define TEAM32_RAY_H

#include "vector.h"

struct Ray {
    // Ray origin
    Vector o;
    // Ray direction
    Vector d;
    // Length of the ray
    float t_max;
};

// point on ray at length t
Vector ray_at(const struct Ray *r, float t);

#endif //TEAM32_RAY_H
