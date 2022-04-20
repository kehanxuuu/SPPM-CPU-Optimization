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

void ray_init(struct Ray* ray, const Vector* o, const Vector* d, const float t_max);

Vector ray_at(const struct Ray *r, const float t); // point on ray at length t

#endif //TEAM32_RAY_H
