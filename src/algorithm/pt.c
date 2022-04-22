#include "pt.h"

void pt_init(PathTracing *pt, int spp, int ray_max_depth, Scene *scene, Camera *camera, Vector background) {
    pt->spp = spp;
    pt->ray_max_depth = ray_max_depth;
    pt->scene = scene;
    pt->camera = camera;
    pt->background = background;
}

void pt_render(PathTracing *pt, Bitmap *film) {
    const size_t W = film->W, H = film->H;
    for (size_t py = 0; py < H; ++py) {
        fprintf(stderr, "\rScanlines remaining %zu out of %zu", H - py, H);
        for (size_t px = 0; px < W; ++px) {
            Vector color = ZERO_VEC;
            for (size_t s = 0; s < pt->spp; ++s) {
                Ray ray = generate_ray(pt->camera, px, py, (Vector2f) {randf(), randf()});
                Vector path_color = radiance(pt->scene, &ray, pt->ray_max_depth, &pt->background);
                vv_addeq(&color, &path_color);
            }
            vs_diveq(&color, (float) pt->spp);

            bitmap_set(film, px, py, &color);
        }
    }
}

// recursive ray tracing
Vector radiance(const Scene *scene, Ray *ray, int depth, const Vector *background) {
    if (depth <= 0)
        return ZERO_VEC;
    Intersection isect;
    if (!scene_intersect(scene, ray, &isect)) {
        return *background;
    }
//    Vector n = vv_add(&isect.n, &(Vector){1, 1, 1});
//    vs_diveq(&n, 2);
//    return n;
//    return isect.hit->albedo;
//    return vs_mul(&isect.hit->albedo, -vv_dot(&isect.wi, &isect.n));

    Vector emitted = isect.hit->emission;

    Vector2f sample = {randf(), randf()};
    Vector attenuation = {0.99f, 0.99f, 0.99f};
    switch (isect.hit->material) {
        case DIFFUSE:
            attenuation = bsdf_sample_diffuse(&isect, sample);
            break;
        case SPECULAR:
            attenuation = bsdf_sample_specular(&isect, sample);
            break;
        default:
            UNIMPLEMENTED;
    }
    if (vv_equal(&attenuation, &ZERO_VEC)) {
        return emitted;
    }
    Ray ray_next_bounce = {isect.p, isect.wo, INFINITY};
    ray_next_bounce.o = ray_at(&ray_next_bounce, EPSILON);
    Vector recursive_color = radiance(scene, &ray_next_bounce, depth - 1, background);
    Vector color = vv_mul(&attenuation, &recursive_color);
    return vv_add(&emitted, &color);
}
