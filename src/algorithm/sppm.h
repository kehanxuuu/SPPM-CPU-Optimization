#ifndef TEAM32_SPPM_H
#define TEAM32_SPPM_H

#include "vector.h"
#include "dt_l.h"
#include "camera.h"
#include "scene.h"
#include "intersection.h"
#include "mesh.h"
#include "array.h"
#include "arrayfloat.h"
#include "arrayint.h"
#include "arraypointer.h"
#include "arrayvector.h"
#include "arrayfixed2d.h"
#include "bitmap.h"
#include "simd_math.h"
#include "simd_warping.h"
#include <immintrin.h>
#include "intersection_l.h"
#include "ray_l.h"

#ifndef _SPPM_RADIUS_MULT
#define _SPPM_RADIUS_MULT 2.0
#endif

struct PixelData {
    int size;
    int size_float_simd;
    FloatL radius;
    FloatL num_photons;

    VectorL tau;
    VectorL direct_radiance;

    // refreshed every iteration
    FloatL cur_photons;
    VectorL cur_flux;

    // struct visible point
    VectorL cur_vp_attenuation;
    IntersectionL cur_vp_intersection; // struct intersection
};

struct PixelDataLookup {
    int fixed_size;
    IntArray *hash_table;
    float inv_grid_res;
    Vector grid_min;
};

struct SPPM {
    int num_iterations;
    int ray_max_depth;
    int num_photons;
    float initial_radius;
    float alpha;
    Vector background;
    Scene *scene;
    Camera *camera;
    float *launch_indices_x;
    float *launch_indices_y;
    float ray_avg_depth;
    float photon_avg_depth;
    float photon_avg_lookups;
};

typedef struct PixelData PixelData;
typedef struct PixelDataLookup PixelDataLookup;
typedef struct SPPM SPPM;

void sppm_init(SPPM *sppm, int num_iterations, int ray_max_depth, int photon_num_iter, float initial_radius, Scene *scene, Camera *camera,
               Vector background);

void sppm_pixel_data_init(PixelData *pixel_datas, int size);

void sppm_pixel_data_lookup_init(PixelDataLookup *lookup, int init_size);

void sppm_pixel_data_lookup_assign(PixelDataLookup *lookup, float grid_size, Vector* grid_min);

void sppm_pixel_data_lookup_clear(PixelDataLookup *lookup);

void sppm_pixel_data_lookup_free(PixelDataLookup *lookup);

void sppm_pixel_data_free(PixelData *pixel_datas);

int sppm_pixel_data_lookup_hash(PixelDataLookup *lookup, int x, int y, int z);

__m256i sppm_pixel_data_lookup_hash_l(PixelDataLookup *lookup, __m256i x, __m256i y, __m256i z);

void sppm_pixel_data_lookup_to_grid(PixelDataLookup *lookup, float loc_x, float loc_y, float loc_z, int* res_x, int *res_y, int* res_z);

void sppm_pixel_data_lookup_to_grid_l(PixelDataLookup *lookup, __m256 loc_x, __m256 loc_y, __m256 loc_z, __m256* ind_x, __m256* ind_y, __m256* ind_z);

void sppm_pixel_data_lookup_store(PixelDataLookup *lookup, int loc_x, int loc_y, int loc_z, int pd_index);

void sppm_build_pixel_data_lookup(PixelDataLookup *lookup, PixelData *pixel_datas);

void sppm_camera_pass(SPPM *sppm, PixelData *pixel_datas);

void sppm_photon_pass(SPPM *sppm, PixelDataLookup *lookup, PixelData *pixel_datas);

void sppm_consolidate(PixelData *pixel_datas, float alpha);

void sppm_store(PixelData *pixel_datas, int num_iters, int num_photons, int H, int W, Bitmap *bitmap);

void sppm_render(SPPM *sppm, Bitmap *bitmap);

#endif //TEAM32_SPPM_H
