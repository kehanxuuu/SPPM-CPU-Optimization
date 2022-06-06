#ifndef TEAM32_VECTOR_H
#define TEAM32_VECTOR_H

#include "common.h"
#include "simd_random.h"

// Basic 3 dimensional vector library
typedef struct {
    float x, y, z;
} Vector3f;

typedef struct {
    size_t x, y, z;
} Vector3u;

typedef Vector3f Vector;

static const Vector ZERO_VEC = {0, 0, 0};
static const Vector UNIT_X = {1, 0, 0};
static const Vector UNIT_Y = {0, 1, 0};
static const Vector UNIT_Z = {0, 0, 1};

typedef struct {
    float x, y;
} Vector2f;

static inline Vector vv_add(const Vector *a, const Vector *b) {
    Vector c = {a->x + b->x, a->y + b->y, a->z + b->z};
    return c;
}

static inline void vv_addeq(Vector *a, const Vector *b) {
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

static inline Vector vv_sub(const Vector *a, const Vector *b) {
    Vector c = {a->x - b->x, a->y - b->y, a->z - b->z};
    return c;
}

static inline void vv_subeq(Vector *a, const Vector *b) {
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

static inline Vector vv_mul(const Vector *a, const Vector *b) {
    Vector c = {a->x * b->x, a->y * b->y, a->z * b->z};
    return c;
}

static inline void vv_muleq(Vector *a, const Vector *b) {
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

static inline Vector vv_div(const Vector *a, const Vector *b) {
    Vector c = {a->x / b->x, a->y / b->y, a->z / b->z};
    return c;
}

static inline void vv_diveq(Vector *a, const Vector *b) {
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

static inline float vv_dot(const Vector *a, const Vector *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline Vector vv_cross(const Vector *a, const Vector *b) {
    Vector c = {
            a->y * b->z - a->z * b->y,
            a->z * b->x - a->x * b->z,
            a->x * b->y - a->y * b->x,
    };
    return c;
}

static inline bool vv_equal(const Vector *a, const Vector *b) {
    const float epsilon = 1e-6f;
    if (fabsf(a->x - b->x) > epsilon)
        return false;
    if (fabsf(a->y - b->y) > epsilon)
        return false;
    if (fabsf(a->z - b->z) > epsilon)
        return false;
    return true;
}

static inline Vector vs_mul(const Vector *a, float s) {
    Vector b = {a->x * s, a->y * s, a->z * s};
    return b;
}

static inline void vs_muleq(Vector *a, float s) {
    a->x *= s;
    a->y *= s;
    a->z *= s;
}

static inline Vector vs_div(const Vector *a, float s) {
    Vector b = {0, 0, 0};
    if (s != 0) {
        b.x = a->x / s;
        b.y = a->y / s;
        b.z = a->z / s;
    }
    return b;
}

static inline void vs_diveq(Vector *a, float s) {
    if (s != 0) {
        a->x /= s;
        a->y /= s;
        a->z /= s;
    }
}

static inline Vector vvv_fma(const Vector *a, const Vector *b, const Vector *c) {
    return (Vector) {
            a->x + b->x * c->x,
            a->y + b->y * c->y,
            a->z + b->z * c->z,
    };
}

static inline void vvv_fmaeq(Vector *a, const Vector *b, const Vector *c) {
    a->x += b->x *c->x;
    a->y += b->y *c->y;
    a->z += b->z *c->z;
}

static inline Vector vvs_fma(const Vector *a, const Vector *b, float s) {
    Vector c = {
            a->x + b->x * s,
            a->y + b->y * s,
            a->z + b->z * s,
    };
    return c;
}

static inline void vvs_fmaeq(Vector *a, const Vector *b, float s) {
    a->x += b->x * s;
    a->y += b->y * s;
    a->z += b->z * s;
}

static inline float v_norm_sqr(const Vector *a) {
    return a->x * a->x + a->y * a->y + a->z * a->z;
}

static inline float v_norm(const Vector *a) {
    return sqrtf(v_norm_sqr(a));
}

static inline void v_normalize(Vector *a) {
    float norm = v_norm(a);
    a->x /= norm;
    a->y /= norm;
    a->z /= norm;
}

static inline Vector v_normalized(const Vector *a) {
    float norm = v_norm(a);
    Vector c = {
            a->x / norm,
            a->y / norm,
            a->z / norm,
    };
    return c;
}

static inline float v_cwise_min(const Vector *a) {
    return (a->x <= a->y && a->x <= a->z) ? a->x : (a->y <= a->z) ? a->y : a->z;
}

static inline float v_cwise_max(const Vector *a) {
    return (a->x >= a->y && a->x >= a->z) ? a->x : (a->y >= a->z) ? a->y : a->z;
}

static inline float v_cwise_sum(const Vector *a) {
    return a->x + a->y + a->z;
}

static inline float v_cwise_mean(const Vector *a) {
    return v_cwise_sum(a) / 3;
}

static inline Vector vv_max(const Vector *a, const Vector *b) {
    return (Vector) {.x = fmaxf(a->x, b->x), .y = fmaxf(a->y, b->y), .z = fmaxf(a->z, b->z)};
}

static inline Vector vv_min(const Vector *a, const Vector *b) {
    return (Vector) {.x = fminf(a->x, b->x), .y = fminf(a->y, b->y), .z = fminf(a->z, b->z)};
}

static inline float randf() {
    float res = (float) simd_rand() / (float) SIMD_RAND_MAX;
    return res;
}
#endif //TEAM32_VECTOR_H
