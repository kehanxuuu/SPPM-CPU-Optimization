#include "vector.h"

Vector square_to_cosine_hemisphere(const Vector2f* sample, const Vector* normal) {
    float phi = 2 * M_PI * sample->y;
    float theta = acosf(sqrtf(sample->x));
    Vector direction;
    direction.x = sinf(theta) * cosf(phi);
    direction.y = sinf(theta) * sinf(phi);
    direction.z = cosf(theta);
    // normal = z axis
    Vector up = {0, 1, 0};
    Vector x_normal = vv_cross(normal, &up);
    Vector y_normal = vv_cross(normal, &x_normal);
    Vector direction_world = vs_mul(&x_normal, direction.x);
    vvs_fmaeq(&direction_world, &y_normal, direction.y);
    vvs_fmaeq(&direction_world, normal, direction.z);
    v_normalize(&direction_world);
    return direction_world;
}

Vector vv_add(const Vector* a, const Vector* b) {
    Vector c = { a->x + b->x, a->y + b->y, a->z + b->z };
    return c;
}

void vv_addeq(Vector* a, const Vector* b) {
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

Vector vv_sub(const Vector* a, const Vector* b) {
    Vector c = { a->x - b->x, a->y - b->y, a->z - b->z };
    return c;
}

void vv_subeq(Vector* a, const Vector* b) {
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

Vector vv_mul(const Vector* a, const Vector* b) {
    Vector c = { a->x * b->x, a->y * b->y, a->z * b->z };
    return c;
}

void vv_muleq(Vector* a, const Vector* b) {
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

Vector vv_div(const Vector* a, const Vector* b) {
    Vector c = { a->x / b->x, a->y / b->y, a->z / b->z };
    return c;
}

void vv_diveq(Vector* a, const Vector* b) {
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

float vv_dot(const Vector* a, const Vector* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

Vector vv_cross(const Vector* a, const Vector* b) {
    Vector c = {
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x,
    };
    return c;
}

bool vv_equal(const Vector *a, const Vector *b) {
    const float epsilon = 1e-6f;
    if (fabsf(a->x - b->x) > epsilon)
        return false;
    if (fabsf(a->y - b->y) > epsilon)
        return false;
    if (fabsf(a->z - b->z) > epsilon)
        return false;
    return true;
}

Vector vs_mul(const Vector* a, float s) {
    Vector b = { a->x * s, a->y * s, a->z * s };
    return b;
}

void vs_muleq(Vector* a, float s) {
    a->x *= s;
    a->y *= s;
    a->z *= s;
}

Vector vs_div(const Vector* a, float s) {
    Vector b = { 0, 0, 0 };
    if (s != 0) {
        b.x = a->x / s;
        b.y = a->y / s;
        b.z = a->z / s;
    }
    return b;
}

void vs_diveq(Vector* a, float s) {
    if (s != 0) {
        a->x /= s;
        a->y /= s;
        a->z /= s;
    }
}

Vector vvs_fma(const Vector* a, const Vector* b, float s) {
    Vector c = {
        a->x + b->x * s,
        a->y + b->y * s,
        a->z + b->z * s,
    };
    return c;
}

void vvs_fmaeq(Vector* a, const Vector* b, float s) {
    a->x += b->x * s;
    a->y += b->y * s;
    a->z += b->z * s;
}

float v_norm_sqr(const Vector* a) {
    return a->x * a->x + a->y * a->y + a->z * a->z;
}

float v_norm(const Vector* a) {
    return sqrtf(v_norm_sqr(a));
}

void v_normalize(Vector* a) {
    float norm = v_norm(a);
    a->x /= norm;
    a->y /= norm;
    a->z /= norm;
}

Vector v_normalized(const Vector* a) {
    float norm = v_norm(a);
    Vector c = {
        a->x / norm,
        a->y / norm,
        a->z / norm,
    };
    return c;
}

float v_cwise_min(const Vector* a) {
    return (a->x <= a->y && a->x <= a->z) ? a->x : (a->y <= a->z) ? a->y : a->z;
}

float v_cwise_max(const Vector* a) {
    return (a->x >= a->y && a->x >= a->z) ? a->x : (a->y >= a->z) ? a->y : a->z;
}

float v_cwise_sum(const Vector* a) {
    return a->x + a->y + a->z;
}

float v_cwise_mean(const Vector* a) {
    return v_cwise_sum(a) / 3;
}

float randf() {
    return (float)rand() / (float)RAND_MAX;
}
