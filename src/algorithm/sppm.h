#ifndef TEAM32_SPPM_H
#define TEAM32_SPPM_H

#include "vector.h"
#include "camera.h"
#include "scene.h"
#include "intersection.h"
#include "mesh.h"
#include "array.h"
#include "arrayfixed2d.h"
#include "bitmap.h"

#ifndef _SPPM_RADIUS_MULT
    #define _SPPM_RADIUS_MULT 2.0
#endif

#ifndef _SPPM_RADIUS_TYPE
    #define _SPPM_RADIUS_TYPE 0
#endif

struct VisiblePoint {
    Intersection intersection;
    Vector attenuation;
};

struct PixelData {
    float radius;
    float num_photons;
    Vector tau;
    Vector direct_radiance;

//    refreshed every iteration
    int cur_photons;
    Vector cur_flux;
    struct VisiblePoint cur_vp;
};

struct PixelDataLookup {
    size_t fixed_size;
    PointerArray *hash_table;
    float grid_res;
    Vector3f grid_min;
    Vector3f grid_max;
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
};

typedef struct VisiblePoint VisiblePoint;
typedef struct PixelData PixelData;
typedef struct PixelDataLookup PixelDataLookup;
typedef struct SPPM SPPM;

void sppm_init(SPPM *sppm, int num_iterations, int ray_max_depth, int photon_num_iter, float initial_radius, Scene *scene, Camera *camera,
               Vector background);

void sppm_pixel_data_lookup_init(PixelDataLookup *lookup, size_t init_size);

void sppm_pixel_data_lookup_assign(PixelDataLookup *lookup, float grid_size, Vector grid_min, Vector grid_max);

void sppm_pixel_data_lookup_clear(PixelDataLookup *lookup);

void sppm_pixel_data_lookup_free(PixelDataLookup *lookup);

size_t sppm_pixel_data_lookup_hash(PixelDataLookup *lookup, Vector3u *loc);

Vector3u sppm_pixel_data_lookup_to_grid(PixelDataLookup *lookup, Vector *loc);

void sppm_pixel_data_lookup_store(PixelDataLookup *lookup, Vector3u *loc_3d, PixelData *pd);

void sppm_build_pixel_data_lookup(PixelDataLookup *lookup, ArrayFixed2D *pixel_datas);

void sppm_camera_pass_pixel(SPPM *sppm, int x, int y, PixelData *pd);

void sppm_camera_pass(SPPM *sppm, ArrayFixed2D *pixel_datas);

void sppm_photon_pass_photon(SPPM *sppm, PixelDataLookup *lookup);

void sppm_photon_pass(SPPM *sppm, PixelDataLookup *lookup);

void sppm_consolidate(SPPM *sppm, ArrayFixed2D *pixel_datas);

void sppm_store(SPPM *sppm, ArrayFixed2D *pixel_datas, int num_iters, Bitmap *bitmap);

void sppm_render(SPPM *sppm, Bitmap *bitmap);

#endif //TEAM32_SPPM_H
