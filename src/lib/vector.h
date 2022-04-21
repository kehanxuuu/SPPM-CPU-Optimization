#ifndef TEAM32_VECTOR_H
#define TEAM32_VECTOR_H

#include "common.h"

// Basic 3 dimensional vector library
typedef struct {
    float x, y, z;
} Vector3f;

typedef struct {
    size_t x, y, z;
} Vector3u;

typedef Vector3f Vector;

static const Vector ZERO_VEC = {0, 0, 0};

typedef struct {
    float x, y;
} Vector2f;

Vector square_to_cosine_hemisphere(const Vector2f* sample, const Vector* normal);  // map [0, 1]x[0, 1] to cosine hemisphere, output direction in world space given normal

Vector vv_add(const Vector *a, const Vector *b);  // vector-vector addition

void vv_addeq(Vector *a, const Vector *b);  // a += b

Vector vv_sub(const Vector *a, const Vector *b);

void vv_subeq(Vector *a, const Vector *b);  // a -= b

Vector vv_mul(const Vector *a, const Vector *b);  // element-wise product

void vv_muleq(Vector *a, const Vector *b);  // a *= b

Vector vv_div(const Vector *a, const Vector *b);  // element-wise division

void vv_diveq(Vector *a, const Vector *b);  // a /= b

float vv_dot(const Vector *a, const Vector *b);

Vector vv_cross(const Vector *a, const Vector *b);

bool vv_equal(const Vector *a, const Vector *b);

Vector vs_mul(const Vector *a, float s);  // vector-scalar multiplication

void vs_muleq(Vector *a, float s);  // a *= s

Vector vs_div(const Vector *a, float s);

void vs_diveq(Vector *a, float s);  // a /= s

Vector vvs_fma(const Vector *a, const Vector *b, float s);  // a + b * s

void vvs_fmaeq(Vector *a, const Vector *b, float s);  // a += b * s

float v_norm(const Vector *a);

float v_norm_sqr(const Vector *a);

void v_normalize(Vector *a);

Vector v_normalized(const Vector *a);

float v_cwise_min(const Vector *a);

float v_cwise_max(const Vector *a);

float v_cwise_sum(const Vector *a);

float v_cwise_mean(const Vector *a);

Vector vv_max(const Vector *a, const Vector *b);

Vector vv_min(const Vector *a, const Vector *b);

// Random float [0, 1)
float randf();

#endif //TEAM32_VECTOR_H
