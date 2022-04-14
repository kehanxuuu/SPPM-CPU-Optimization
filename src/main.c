#include "scene.h"
#include "camera.h"
#include "bitmap.h"

int main() {
    srand(0);
    size_t W = 1920, H = 1080;
    struct Bitmap film;
    bitmap_init(&film, W, H);

    struct Scene scene;
    {
        // TODO: scene initialization codes
    }

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
            // scene_intersect(&scene, &ray, &isect);
            Vector color = {(float) px / (float) W, (float) py / (float) H, 0};
            bitmap_set(&film, px, py, &color);
        }
    }

    bitmap_save_exr(&film, "../out/sample.exr");
    bitmap_free(&film);
    return 0;
}
