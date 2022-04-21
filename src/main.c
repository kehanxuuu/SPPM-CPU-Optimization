#include "scene.h"
#include "camera.h"
#include "bitmap.h"
#include "sppm.h"

// recursive ray tracing
Vector ray_color(const Scene *scene, Ray* ray, int depth, const Vector *background) {
    if (depth <= 0)
        return ZERO_VEC;
    Intersection isect;
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
    size_t W = 500, H = 500;

    // TODO: scene initialization codes
    // Simple scene & initialization, just for test
    Vector v1 = {0, 0, 0}, v2 = {2, 2, -7}, v3 = {-5, -5, -1};
    float r1 = 3, r2 = 4, r3 = 2;
    Vector albedo1 = {0.5, 0.5, 0.5}, albedo2 = {0.75, 1.0, 0.75}, albedo3 = {1.0, 0.96, 0.0};
    Vector emission1 = {0.4, 0, 0}, emission2 = ZERO_VEC, emission3 = ZERO_VEC;
    Mesh mesh1, mesh2, mesh3;
    mesh_init_sphere(&mesh1, v1, r1, DIFFUSE, albedo1, emission1);
    mesh_init_sphere(&mesh2, v2, r2, DIFFUSE, albedo2, emission2);
    mesh_init_sphere(&mesh3, v3, r3, SPECULAR, albedo3, emission3);
    Scene scene;
    scene_init_with_mesh(&scene, &mesh1);
    scene_add(&scene, &mesh2);
    scene_add(&scene, &mesh3);
    Vector background = {0.5, 0.7, 1.0}; // mimic sky color for now, should be zero for physical correctness

    struct Camera camera;
    {
        cam_set_resolution(&camera, W, H);
        camera.fov = M_PI_2;
        Vector eye = {0, 0, 10}, target = {0, 0, 0}, up = {0, 1, 0};
        cam_look_at(&camera, &eye, &target, &up);
    }

    SPPM sppm;
    sppm_init(&sppm, 100, 20, 100000, 0.1f, &scene, &camera, &background);
    Bitmap film;
    bitmap_init(&film, W, H);
    sppm_render(&sppm, &film);

    bitmap_save_exr(&film, "../../out/sample.exr");
    bitmap_free(&film);

    scene_free(&scene);

    sppm_free(&sppm);
    printf("Safe exit\n");
    return 0;
}
