#include "scene.h"
#include "camera.h"
#include "bitmap.h"

int main() {
    srand(0);
    size_t W = 1920, H = 1080;
    struct Bitmap film;
    bitmap_init(&film, W, H);
    
    // TODO: scene initialization codes
    // Simple scene & initialization, just for test
    Vector v1 = {0, 0, 0}, v2 = {2, 2, 0};
    float r1 = 3, r2 = 2;
    struct Mesh *mesh1 = mesh_init_sphere(v1, r1);
    struct Mesh *mesh2 = mesh_init_sphere(v2, r2);
    struct Scene *scene = scene_init_with_mesh(mesh1);
    scene_add(scene, mesh2);

    struct Camera camera;
    {
        cam_set_resolution(&camera, W, H);
        camera.fov = M_PI_2;
        Vector eye = {0, 0, 10}, target = {0, 0, 0}, up = {0, 1, 0};
        cam_look_at(&camera, &eye, &target, &up);
    }

    for (size_t py = 0; py < H; ++py) {
        for (size_t px = 0; px < W; ++px) {
            Vector2f sample = {randf(), randf()};
            Ray ray = generate_ray(&camera, px, py, sample);
            struct Intersection isect;
            Vector color;
            if (scene_intersect(scene, &ray, &isect)) {
                color = (Vector) {1, 1, 1};
            }
            else
            {
                color = (Vector) {(float) px / (float) W, (float) py / (float) H, 0};
            }
            bitmap_set(&film, px, py, &color);
        }
    }

    bitmap_save_exr(&film, "../out/sample.exr");
    bitmap_free(&film);

    scene_free(scene);

    return 0;
}
