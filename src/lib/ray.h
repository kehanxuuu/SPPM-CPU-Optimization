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
static inline Vector ray_at(const struct Ray *r, float t) { return vvs_fma(&r->o, &r->d, t); }

#endif //TEAM32_RAY_H
