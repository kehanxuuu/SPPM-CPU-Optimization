#include "scene.h"
#include "camera.h"
#include "bitmap.h"

// recursive ray tracing
Vector ray_color(const struct Scene *scene, Ray* ray, int depth, const Vector *background) {
    if (depth <= 0)
        return ZERO_VEC;
    struct Intersection isect;
    if (!scene_intersect(scene, ray, &isect)) {
        return *background;
    }

    Vector emitted = isect.hit->emission;

    Vector2f sample = {randf(), randf()};
    Vector attenuation = ZERO_VEC;
    switch (isect.hit->material) {
        case DIFFUSE:
            attenuation = bsdf_sample_diffuse(&isect, sample);
            break;
        case SPECULAR:
            attenuation = bsdf_sample_specular(&isect, sample);
            break;
        default:
            break;
    }
    if (vv_equal(&attenuation, &ZERO_VEC)) {
        return emitted;
    }
    Ray ray_next_bounce = {isect.p, isect.wo, INFINITY};
    Vector recursive_color = ray_color(scene, &ray_next_bounce, depth - 1, background);
    Vector color = vv_mul(&attenuation, &recursive_color);
    return vv_add(&emitted, &color);
}

int main() {
    srand(0);
    size_t W = 1920, H = 1080;
    struct Bitmap film;
    bitmap_init(&film, W, H);

    // TODO: scene initialization codes
    // Simple scene & initialization, just for test
    Vector v1 = {0, 0, 0}, v2 = {2, 2, -7}, v3 = {-5, -5, -1};
    float r1 = 3, r2 = 4, r3 = 2;
    Vector albedo1 = {0.5, 0.5, 0.5}, albedo2 = {0.75, 1.0, 0.75}, albedo3 = {1.0, 0.96, 0.0};
    Vector emission1 = {0.4, 0, 0}, emission2 = ZERO_VEC, emission3 = ZERO_VEC;
    struct Mesh mesh1, mesh2, mesh3;
    mesh_init_sphere(&mesh1, v1, r1, DIFFUSE, albedo1, emission1);
    mesh_init_sphere(&mesh2, v2, r2, DIFFUSE, albedo2, emission2);
    mesh_init_sphere(&mesh3, v3, r3, SPECULAR, albedo3, emission3);
    struct Scene scene;
    scene_init_with_mesh(&scene, &mesh1);
    scene_add(&scene, &mesh2);
    scene_add(&scene, &mesh3);

    struct Camera camera;
    {
        cam_set_resolution(&camera, W, H);
        camera.fov = M_PI_2;
        Vector eye = {0, 0, 10}, target = {0, 0, 0}, up = {0, 1, 0};
        cam_look_at(&camera, &eye, &target, &up);
    }

    const size_t SPP = 5; // samples per pixel
    Vector background = {0.5, 0.7, 1.0}; // mimic sky color for now, should be zero for physical correctness

    for (size_t py = 0; py < H; ++py) {
        fprintf(stderr, "\rScanlines remaining %zu out of %zu", H - py, H);
        for (size_t px = 0; px < W; ++px) {
            Vector color = ZERO_VEC;
            /* intersection test code */
            // Vector2f sample = {randf(), randf()};
            // Ray ray = generate_ray(&camera, px, py, sample);
            // struct Intersection isect;
            // if (scene_intersect(scene, &ray, &isect)) {
            //     color = (Vector) {1, 1, 1};
            // }
            // else
            // {
            //     color = (Vector) {(float) px / (float) W, (float) py / (float) H, 0};
            // }
            /* intersection test code ends */

            /* ray tracing test code */
            for (size_t s = 0; s < SPP; ++s) {
                Vector2f sample = {randf(), randf()};
                Ray ray = generate_ray(&camera, px, py, sample);
                Vector light_path_color = ray_color(&scene, &ray, 5, &background);
                vv_addeq(&color, &light_path_color);
            }
            vs_diveq(&color, (float)SPP);
            /* ray tracing test code ends */

            bitmap_set(&film, px, py, &color);
        }
    }

    bitmap_save_exr(&film, "sample.exr");
    bitmap_free(&film);

    scene_free(&scene);
    printf("Safe exit\n");
    return 0;
}
