#ifndef TEAM32_PT_H
#define TEAM32_PT_H

#include "camera.h"
#include "scene.h"
#include "bitmap.h"

struct PathTracing {
    int spp;
    int ray_max_depth;
    Vector background;
//    struct Emitters emitters;
    Scene *scene;
    Camera *camera;
};

typedef struct PathTracing PathTracing;

Vector radiance(const Scene *scene, Ray *ray, int max_depth, const Vector *background);

void pt_init(PathTracing *pt, int spp, int ray_max_depth, Scene *scene, Camera *camera, Vector background);

void pt_render(PathTracing *pt, Bitmap *film);

#endif //TEAM32_PT_H
