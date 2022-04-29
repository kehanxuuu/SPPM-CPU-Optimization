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
Vector radiance(const Scene *scene, Ray *ray, int max_depth, const Vector *background) {
    Vector beta = {0.99f, 0.99f, 0.99f};  // throughput
    Vector L = ZERO_VEC;  // radiance
    bool after_NEE = false;

    while (true) {
        Intersection isect;
        if (!scene_intersect(scene, ray, &isect)) {
            vvv_fmaeq(&L, &beta, background);  // L += beta * background
            break;
        }

        if (!after_NEE) {
            vvv_fmaeq(&L, &beta, &isect.hit->emission);  // L += beta * emission
        }

        if (isect.hit->material == DIFFUSE) {
            // Next event estimation
            Vector Ld = estimate_direct_lighting(scene, &isect);
            vvv_fmaeq(&L, &beta, &Ld);  // L += beta * direct_lighting
            after_NEE = true;
            break;
        } else {
            after_NEE = false;
        }

        Vector attenuation = bsdf_sample(&isect, (Vector2f) {randf(), randf()});
        vv_muleq(&beta, &attenuation);

        if (--max_depth <= 0 || vv_equal(&beta, &ZERO_VEC)) {
            break;
        }

        // Russian Roulette
        float max_beta = v_cwise_max(&beta);
        if (max_beta <= 0.25) {
            float P_contd = fminf(max_beta, 0.99f);
            if (randf() <= P_contd) {
                vs_diveq(&beta, P_contd);
            }
            else {
                break;
            }
        }

        *ray = (Ray) {isect.p, isect.wo, INFINITY};
        ray->o = ray_at(ray, EPSILON);
    }

    return L;
}
