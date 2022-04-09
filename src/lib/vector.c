#include "vector.h"

Vector vv_sub(const Vector *a, const Vector *b) {
    // TODO: implement this
}

Vector vs_mul(const Vector *a, float s) {
    Vector b = {a->x * s, a->y * s, a->z * s};
    return b;
}

void vvs_fmaeq(Vector *a, const Vector *b, float s) {
    a->x += b->x * s;
    a->y += b->y * s;
    a->z += b->z * s;
}

Vector vv_cross(const Vector *a, const Vector *b) {
    // TODO: implement this
}

void v_normalize(Vector *a) {
    // TODO: implement this
}

float randf() {
    return (float) rand() / (float) RAND_MAX;
}
