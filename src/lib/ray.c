#include "ray.h"

void ray_init(struct Ray* ray, const Vector* o, const Vector* d, const float t_max){
    ray->o = *o;
    ray->d = *d;
    ray->t_max = t_max;
}

Vector ray_at(const struct Ray *r, const float t) {
    return vvs_fma(&r->o, &r->d, t);
}