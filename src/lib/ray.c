#include "ray.h"

void ray_init(struct Ray* ray, const Vector* o, const Vector* d, const float t_max){
    ray->o = *o;
    ray->d = *d;
    ray->t_max = t_max;
}

Vector ray_at(const struct Ray *r, const float t) {
    Vector dt = vs_mul(&r->d, t);
    return vv_add(&r->o, &dt);
}