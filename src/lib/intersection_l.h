#ifndef TEAM32_INTERSECTION_L_H
#define TEAM32_INTERSECTION_L_H

#ifndef TEAM32_INTERSECTION_H
#define TEAM32_INTERSECTION_H

#include "vector.h"
#include "dt_l.h"
#include "mesh.h"
#include "simd_warping.h"

// This struct describes a surface intersection
struct IntersectionL {
    FloatL mesh_material;
    VectorL mesh_albedo;
    VectorL mesh_emission;
    FloatL mesh_ir;
    VectorL p;    // intersection point
    VectorL n;    // intersection normal for shading
    VectorL wi;  // incident direction (in world frame)
    VectorL wo;  // excident direction
    FloatL interior; // whether intersects at mesh interior
};

// generate samples internally to avoid moves
void bsdf_sample_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_diffuse_l(struct IntersectionL* isects, size_t ind, __m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_diffuse_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_specular_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_specular_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_dielectric_l(struct IntersectionL* isects, size_t ind, __m256 samples0, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_dielectric_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);


#endif //TEAM32_INTERSECTION_H


#endif //TEAM32_INTERSECTION_L_H
