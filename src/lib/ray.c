#include "ray.h"


Vector ray_at(const struct Ray *r, float t) { return vvs_fma(&r->o, &r->d, t); }