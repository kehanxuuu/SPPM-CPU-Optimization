#ifndef TEAM32_NORMAL_H
#define TEAM32_NORMAL_H

#include "camera.h"
#include "scene.h"
#include "bitmap.h"

struct NormalVisualizer {
    Vector background;
    Scene *scene;
    Camera *camera;
};

typedef struct NormalVisualizer NormalVisualizer;

void nv_init(NormalVisualizer *nv, Scene *scene, Camera *camera, Vector background);

void nv_render(NormalVisualizer *nv, Bitmap *film);

#endif //TEAM32_NORMAL_H
