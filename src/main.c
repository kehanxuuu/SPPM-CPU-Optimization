#include "scene.h"
#include "camera.h"
#include "bitmap.h"
#include "sppm.h"
#include "pt.h"

void init_cornell_box(Scene *scene, Camera *camera) {
    const float inf = 1e4f;
    scene_init(scene);
    Mesh *left = mesh_make_sphere((Vector) {inf + 1, 40.8f, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .25f, .25f}, ZERO_VEC, 1.0);
    Mesh *right = mesh_make_sphere((Vector) {-inf + 99, 40.8f, 81.6f}, inf, DIFFUSE, (Vector) {.25f, .25f, .75f}, ZERO_VEC, 1.0);
    Mesh *back = mesh_make_sphere((Vector) {50, 40.8f, inf}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
    Mesh *front = mesh_make_sphere((Vector) {50, 40.8f, -inf + 170}, inf, DIFFUSE, ZERO_VEC, ZERO_VEC, 1.0);
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

int main() {
    srand(0);
    size_t W = 1024, H = 768;

    Scene scene;
    Camera camera;
    init_cornell_box(&scene, &camera);
    cam_set_resolution(&camera, W, H);

    Bitmap film;
    bitmap_init(&film, W, H);

    Vector background = ZERO_VEC; // mimic sky color for now, should be zero for physical correctness

//    PathTracing pt;
//    pt_init(&pt, 16, 10, &scene, &camera, background);
//    pt_render(&pt, &film);
    SPPM sppm;
    sppm_init(&sppm, 8, 20, 200000, 2.0f, &scene, &camera, background);
    sppm_render(&sppm, &film);

    bitmap_save_exr(&film, "../../out/cornell-sppm.exr");
    bitmap_free(&film);

    scene_free(&scene);

//    sppm_free(&sppm);
    printf("Safe exit\n");
    return 0;
}
