#include "camera.h"

void cam_look_at(struct Camera *camera, const Vector *eye, const Vector *target, const Vector *up) {
    camera->c = *eye;
    camera->ez = vv_sub(target, eye);  // front
    camera->ex = vv_cross(&camera->ez, up);  // right
    camera->ey = vv_cross(&camera->ez, &camera->ex);  // down
    v_normalize(&camera->ex);
    v_normalize(&camera->ey);
    v_normalize(&camera->ez);
}

void cam_set_resolution(struct Camera *camera, size_t W, size_t H) {
    camera->W = W;
    camera->H = H;
    camera->aspect = (float) W / (float) H;
}

struct Ray generate_ray(const struct Camera * camera, size_t px, size_t py, Vector2f sample) {
    float tg = tanf(camera->fov / 2.0f);
    Vector dc = {
            camera->aspect * tg * (2 * ((float) px + sample.x) / (float) camera->W - 1),
            tg * (2 * ((float) py + sample.y) / (float) camera->H - 1),
            1
    };
    Vector d = vs_mul(&camera->ex, dc.x);
    vvs_fmaeq(&d, &camera->ey, dc.y);
    vvs_fmaeq(&d, &camera->ez, dc.z);
    v_normalize(&d);

    struct Ray ray;
    ray_init(&ray, &camera->c, &d, INFINITY);
    return ray;
}
