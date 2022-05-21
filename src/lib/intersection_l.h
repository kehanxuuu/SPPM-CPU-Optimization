#ifndef TEAM32_INTERSECTION_L_H
#define TEAM32_INTERSECTION_L_H

#include "vector.h"
#include "dt_l.h"
#include "mesh.h"
#include "scene.h"
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

typedef struct{
    FloatM mesh_material;
    VectorM mesh_albedo;
    VectorM mesh_emission;
    FloatM mesh_ir;
    VectorM p;    // intersection point
    VectorM n;    // intersection normal for shading
    VectorM wi;  // incident direction (in world frame)
    VectorM wo;  // excident direction
    FloatM interior; // whether intersects at mesh interior
} __attribute__((__aligned__(64))) IntersectionM;

void intersection_l_init(IntersectionL *isects, size_t size);

void intersection_l_free(IntersectionL *isects);

// generate samples internally to avoid moves
void bsdf_sample_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_diffuse_m(IntersectionM* isects, __m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_diffuse_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_specular_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_specular_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_sample_dielectric_m(IntersectionM* isects, __m256 samples0, __m256* res_x, __m256* res_y, __m256* res_z);

void bsdf_eval_dielectric_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z);

__m256 scene_intersect_l(struct Scene *scene, __m256 ray_o_x, __m256 ray_o_y, __m256 ray_o_z, __m256 ray_d_x, __m256 ray_d_y, __m256 ray_d_z, __m256* ray_t_max, IntersectionM* isect);

__m256 scene_do_intersect_l(struct Scene *scene, __m256 ray_o_x, __m256 ray_o_y, __m256 ray_o_z, __m256 ray_d_x, __m256 ray_d_y, __m256 ray_d_z, __m256 ray_t_max);
#endif //TEAM32_INTERSECTION_L_H
