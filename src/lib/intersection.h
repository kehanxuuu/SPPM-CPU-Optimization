#ifndef TEAM32_INTERSECTION_H
#define TEAM32_INTERSECTION_H

#include "vector.h"

// This struct describes a surface intersection
struct Intersection {
    struct Mesh *hit;  // intersected mesh
    Vector p;    // intersection point
    Vector n;    // intersection normal for shading
    Vector wi;  // incident direction (in world frame)
    Vector wo;  // excident direction
};

// TODO: for the implementation of this section, please refer to `Nori/bsdf.h`

// Sample the BSDF of `isect->hit` and return the importance weight (i.e. the value of the BSDF * albedo * cos(theta_o) divided by the probability density
// of the sample with respect to solid angles). The sampled direction is stored to `isect->wo`
Vector bsdf_sample(struct Intersection *isect, Vector2f sample);

// Return the BSDF(wi, wo) * albedo
Vector bsdf_eval(struct Intersection *isect);

// This method provides access to the probability density that is realized by the bsdf_sample() method.
float bsdf_pdf(struct Intersection *isect);

#endif //TEAM32_INTERSECTION_H
