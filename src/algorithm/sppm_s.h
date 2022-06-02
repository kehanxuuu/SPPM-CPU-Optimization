#ifndef TEAM32_SPPM_S_H
#define TEAM32_SPPM_S_H

#include "vector.h"
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

#ifndef _SPPM_RADIUS_MULT
#define _SPPM_RADIUS_MULT 2.0
#endif

struct PixelDataS {
    FloatArray radius;
    FloatArray num_photons;
    VectorArray tau;
    VectorArray direct_radiance;

    // refreshed every iteration
    FloatArray cur_photons;
    VectorArray cur_flux;

    // struct visible point
    VectorArray cur_vp_attenuation;
    Array cur_vp_intersection; // struct intersection
};

struct PixelDataLookupS {
    size_t fixed_size;
    IntArray *hash_table;
    float grid_res;
    Vector3f grid_min;
    Vector3f grid_max;
};

struct SPPM_S {
    int num_iterations;
    int ray_max_depth;
    int num_photons;
    float initial_radius;
    float alpha;
    Vector background;
    Scene *scene;
    Camera *camera;
    float ray_avg_depth;
};

typedef struct PixelDataS PixelDataS;
typedef struct PixelDataLookupS PixelDataLookupS;
typedef struct SPPM_S SPPM_S;

void sppm_init_s(SPPM_S *sppm, int num_iterations, int ray_max_depth, int photon_num_iter, float initial_radius, Scene *scene, Camera *camera,
               Vector background);

void sppm_pixel_data_init_s();

void sppm_pixel_data_lookup_init_s(PixelDataLookupS *lookup, size_t init_size);

void sppm_pixel_data_lookup_assign_s(PixelDataLookupS *lookup, float grid_size, Vector grid_min, Vector grid_max);

void sppm_pixel_data_lookup_clear_s(PixelDataLookupS *lookup);

void sppm_pixel_data_lookup_free_s(PixelDataLookupS *lookup);

void sppm_pixel_data_free_s(PixelDataS *pixel_datas);

size_t sppm_pixel_data_lookup_hash_s(PixelDataLookupS *lookup, Vector3u *loc);

Vector3u sppm_pixel_data_lookup_to_grid_s(PixelDataLookupS *lookup, Vector *loc);

void sppm_pixel_data_lookup_store_s(PixelDataLookupS *lookup, Vector3u *loc_3d, int pd_index);

void sppm_build_pixel_data_lookup_s(PixelDataLookupS *lookup, PixelDataS *pixel_datas, size_t H, size_t W);

void sppm_camera_pass_pixel_s(SPPM_S *sppm, int x, int y, Vector* direct_radiance, Vector* vp_attenuation, Intersection* vp_intersection);

void sppm_camera_pass_s(SPPM_S *sppm, PixelDataS *pixel_datas);

void sppm_photon_pass_photon_s(SPPM_S *sppm, PixelDataLookupS *lookup, PixelDataS *pixel_datas);

void sppm_photon_pass_s(SPPM_S *sppm, PixelDataLookupS *lookup, PixelDataS *pixel_datas);

void sppm_consolidate_s(PixelDataS *pixel_datas, float alpha, size_t H, size_t W);

void sppm_store_s(PixelDataS *pixel_datas, int num_iters, int num_photons, size_t H, size_t W, Bitmap *bitmap);

void sppm_render_s(SPPM_S *sppm, Bitmap *bitmap);

#endif //TEAM32_SPPM_S_H
