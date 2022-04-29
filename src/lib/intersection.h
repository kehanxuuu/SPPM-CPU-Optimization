#ifndef TEAM32_INTERSECTION_H
#define TEAM32_INTERSECTION_H

#include "vector.h"
#include "mesh.h"

// This struct describes a surface intersection
struct Intersection {
    struct Mesh *hit;  // intersected mesh
    Vector p;    // intersection point
    Vector n;    // intersection normal for shading
    Vector wi;  // incident direction (in world frame)
    Vector wo;  // excident direction
    bool interior; // whether intersects at mesh interior
};

// TODO: for the implementation of this section, please refer to `Nori/bsdf.h`

// Sample the BSDF of `isect->hit` and return the importance weight (i.e. the value of the BSDF * albedo * cos(theta_o) divided by the probability density
// of the sample with respect to solid angles). The sampled direction is stored to `isect->wo`
Vector bsdf_sample(struct Intersection *isect, Vector2f sample);

// Return the BSDF(wi, wo) * albedo
Vector bsdf_eval(const struct Intersection *isect);

// This method provides access to the probability density that is realized by the bsdf_sample() method.
float bsdf_pdf(struct Intersection *isect);

// 1. create separate bsdf function for each material (Is that good?)
// !! 2. unless the material is microfacet, eval & pdf function are useless, only sample is used
// TODO: Discuss

// diffuse
Vector bsdf_sample_diffuse(struct Intersection *isect, Vector2f sample);

Vector bsdf_eval_diffuse(const struct Intersection *isect);

float bsdf_pdf_diffuse(struct Intersection *isect);

// specular -> correspond to mirror in Nori, not conductor (so not considering fresnel), but add albedo
Vector bsdf_sample_specular(struct Intersection *isect);

Vector bsdf_eval_specular(const struct Intersection *isect);

float bsdf_pdf_specular(struct Intersection *isect);

// dielectic
Vector bsdf_sample_dielectic(struct Intersection *isect, float sample);

Vector bsdf_eval_dielectic(const struct Intersection *isect);

float bsdf_pdf_dielectic(struct Intersection *isect);

#endif //TEAM32_INTERSECTION_H
