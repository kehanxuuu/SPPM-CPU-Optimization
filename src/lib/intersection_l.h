#ifndef TEAM32_INTERSECTION_L_H
#define TEAM32_INTERSECTION_L_H

#include "vector.h"
#include "dt_l.h"
#include "mesh.h"
#include "simd_warping.h"

// This struct describes a surface intersection
typedef struct{
    FloatL mesh_material;
    VectorL mesh_albedo;
    VectorL mesh_emission;
    FloatL mesh_ir;
    VectorL p;    // intersection point
    VectorL n;    // intersection normal for shading
    VectorL wi;  // incident direction (in world frame)
    VectorL wo;  // excident direction
    FloatL interior; // whether intersects at mesh interior
} IntersectionL;

void intersection_l_init(IntersectionL *isects, size_t size);

void intersection_l_free(IntersectionL *isects);

// generate samples internally to avoid moves
void bsdf_sample_l(IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_l(IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_diffuse_l(IntersectionL* isects, size_t ind, __m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_diffuse_l(IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_specular_l(IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_specular_l(IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_dielectric_l(IntersectionL* isects, size_t ind, __m256 samples0, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_dielectric_l(IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z);

#endif //TEAM32_INTERSECTION_L_H
