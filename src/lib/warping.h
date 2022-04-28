#ifndef TEAM32_WARPING_H
#define TEAM32_WARPING_H

#include "vector.h"

Vector2f square_to_uniform_disk(Vector2f sample);

// map [0, 1]x[0, 1] to cosine hemisphere, output direction in world space given normal
Vector square_to_cosine_hemisphere(Vector2f sample, const Vector *normal);

Vector square_to_uniform_sphere(Vector2f sample);

// sample the cone towards world space direction ez
Vector square_to_uniform_cone(Vector2f sample, float cos_theta_max, const Vector *ez);

// return the probability of reflection
float fresnel(float costheta1, float n1, float n2);

#endif //TEAM32_WARPING_H
