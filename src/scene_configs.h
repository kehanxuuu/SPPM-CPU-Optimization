#ifndef TEAM32_SCENE_CONFIGS_H
#define TEAM32_SCENE_CONFIGS_H

#include "scene.h"
#include "camera.h"

void init_cornell_box(Scene *scene, Camera *camera) {
    const float inf = 1e4f;
    scene_init(scene);
    Mesh *left = mesh_make_sphere((Vector) {inf + 1, 40.8f, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .25f, .25f}, ZERO_VEC, 1.0);
    Mesh *right = mesh_make_sphere((Vector) {-inf + 99, 40.8f, 81.6f}, inf, DIFFUSE, (Vector) {.25f, .25f, .75f}, ZERO_VEC, 1.0);
    Mesh *back = mesh_make_sphere((Vector) {50, 40.8f, inf}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
//    Mesh *front = mesh_make_sphere((Vector) {50, 40.8f, -inf + 170}, inf, DIFFUSE, ZERO_VEC, ZERO_VEC, 1.0);
    Mesh *bottom = mesh_make_sphere((Vector) {50, inf, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
    Mesh *top = mesh_make_sphere((Vector) {50, -inf + 81.6f, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
    Mesh *mirror = mesh_make_sphere((Vector) {27, 16.5f, 47}, 16.5f, SPECULAR, (Vector) {.999f, .999f, .999f}, ZERO_VEC, 1.0);
    Mesh *glass = mesh_make_sphere((Vector) {73, 16.5f, 78}, 16.5f, DIELECTRIC, (Vector) {.999f, .999f, .999f}, ZERO_VEC, 1.5);
    Mesh *light1 = mesh_make_sphere((Vector) {20, 81.6f - 16.5f, 81.6f}, 7.5f, DIFFUSE, ZERO_VEC, (Vector) {20, 10, 10}, 1.0);
    Mesh *light2 = mesh_make_sphere((Vector) {80, 81.6f - 16.5f, 81.6f}, 5, DIFFUSE, ZERO_VEC, (Vector) {10, 10, 20}, 1.0);
    scene_add(scene, left);
    scene_add(scene, right);
    scene_add(scene, back);
//    scene_add(scene, front);
    scene_add(scene, bottom);
    scene_add(scene, top);
    scene_add(scene, mirror);
    scene_add(scene, glass);
    scene_add(scene, light1);
    scene_add(scene, light2);
    scene_finish(scene);
    camera->fov = 30 * M_PI / 180.f;
    Vector eye = {50, 52, 295.6f}, target = vv_add(&eye, &(Vector) {0, -0.042612f, -1}), up = {0, 1, 0};
    cam_look_at(camera, eye, target, up);
}

void init_sky(Scene *scene, Camera *camera) {
    scene_init(scene);
    Mesh *sun = mesh_make_sphere((Vector) {3000, 0, 6000}, 1600, DIFFUSE, ZERO_VEC,
                                 (Vector) {1.2e1 * 1.56 * 2, 0.9 * 1.2e1 * 1.56 * 2, 0.8 * 1.2e1 * 1.56 * 2}, 1.0);
    Mesh *horizon_1 = mesh_make_sphere((Vector) {3500, 0, 7000}, 1560, DIFFUSE, ZERO_VEC,
                                       (Vector) {4.8e1 * 1.56 * 2, 0.5 * 4.8e1 * 1.56 * 2, 0.05 * 4.8e1 * 1.56 * 2}, 1.0);
    Mesh *sky = mesh_make_sphere((Vector) {50, 40.8, 62 - 200}, 10000, DIFFUSE, (Vector) {.7 * .25, .7 * .25, 1 * .25},
                                 (Vector) {0.00063842 * 6e-2 * 8, 0.02001478 * 6e-2 * 8, 0.28923243 * 6e-2 * 8}, 1.0);
    Mesh *ground = mesh_make_sphere((Vector) {50, -10000, 0}, 10000, DIFFUSE,
                                    (Vector) {.3, .3, .3}, ZERO_VEC, 1.0);
    Mesh *horizon_2 = mesh_make_sphere((Vector) {50, -11005, 0}, 11000, DIFFUSE,
                                       ZERO_VEC, (Vector) {.9 * 4, .5 * 4, .05 * 4}, 1.0);
    Mesh *mountains = mesh_make_sphere((Vector) {50, -1e4 - 80, -3000}, 1e4, DIFFUSE,
                                       (Vector) {.2, .2, .2}, ZERO_VEC, 1.0);
    Mesh *white_mirror = mesh_make_sphere((Vector) {22, 26.5, 42}, 26.5, SPECULAR,
                                          (Vector) {.596, .596, .596}, ZERO_VEC, 1.0);
    Mesh *glass_1 = mesh_make_sphere((Vector) {75, 13, 82}, 13, DIELECTRIC,
                                     (Vector) {.96 * .96, .96 * .96, .96 * .96}, ZERO_VEC, 1.5);
    Mesh *glass_2 = mesh_make_sphere((Vector) {87, 22, 24}, 22, DIELECTRIC,
                                     (Vector) {.6 * .696, .6 * .696, .6 * .696}, ZERO_VEC, 1.5);
    scene_add(scene, sun);
    scene_add(scene, horizon_1);
    scene_add(scene, sky);
    scene_add(scene, ground);
    scene_add(scene, horizon_2);
    scene_add(scene, mountains);
    scene_add(scene, white_mirror);
    scene_add(scene, glass_1);
    scene_add(scene, glass_2);
    scene_finish(scene);
    camera->fov = 30 * M_PI / 180.f;
    Vector eye = {50, 52, 295.6f}, target = vv_add(&eye, &(Vector) {0, -0.042612f, -1}), up = {0, 1, 0};
    cam_look_at(camera, eye, target, up);
}

#endif //TEAM32_SCENE_CONFIGS_H
