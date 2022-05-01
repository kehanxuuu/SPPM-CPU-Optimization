#include "normal.h"

void nv_init(NormalVisualizer *nv, Scene *scene, Camera *camera, Vector background) {
    nv->scene = scene;
    nv->camera = camera;
    nv->background = background;
}

void nv_render(NormalVisualizer *nv, Bitmap *film) {
    const size_t W = film->W, H = film->H;
    const float AA_dx[4] = {0.25f, 0.25f, 0.75f, 0.75f};
    const float AA_dy[4] = {0.25f, 0.75f, 0.25f, 0.75f};
    for (size_t py = 0; py < H; ++py) {
        fprintf(stderr, "\rScanlines remaining %zu out of %zu", H - py, H);
        for (size_t px = 0; px < W; ++px) {
            Vector color = ZERO_VEC;
            for (size_t s = 0; s < 4; ++s) {
                Ray ray = generate_ray(nv->camera, px, py, (Vector2f) {AA_dx[s], AA_dy[s]});
                Intersection isect;
                Vector path_color;
                if (!scene_intersect(nv->scene, &ray, &isect)) {
                    path_color = nv->background;
                }
                else {
                    path_color = vvs_fma(&(Vector) {0.5f, 0.5f, 0.5f}, &isect.n, 0.5f);  // 0.5 + n/2
                }
                vv_addeq(&color, &path_color);
            }
            vs_diveq(&color, 4);
            bitmap_set(film, px, py, &color);
        }
    }
}