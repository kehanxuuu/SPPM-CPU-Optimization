#include "warping.h"

Vector2f square_to_uniform_disk(Vector2f sample) {
    float phi = 2 * M_PI * sample.x, r = sqrtf(sample.y);
    return (Vector2f) {r * cosf(phi), r * sinf(phi)};
}

static Vector to_world(const Vector *a, const Vector *ez) {
    // Find onb
    Vector ex = fabsf(vv_dot(ez, &UNIT_Y)) < 0.9 ? vv_cross(ez, &UNIT_Y) : vv_cross(ez, &UNIT_Z);
    v_normalize(&ex);
    Vector ey = vv_cross(ez, &ex);
    Vector b = vs_mul(&ex, a->x);
    vvs_fmaeq(&b, &ey, a->y);
    vvs_fmaeq(&b, ez, a->z);
    return b;
}

Vector square_to_cosine_hemisphere(Vector2f sample, const Vector *ez) {
    union {
        Vector2f xy;
        Vector3f xyz;
    } d;
    d.xy = square_to_uniform_disk(sample);
    d.xyz.z = sqrtf(fmaxf(0.0f, 1 - d.xy.x * d.xy.x - d.xy.y * d.xy.y));
    return to_world(&d.xyz, ez);
}

Vector square_to_uniform_hemisphere(Vector2f sample, const Vector *ez) {
    float sin_theta = sqrtf(fmaxf(0.0f, 1 - sample.x * sample.x));
    float phi = sample.y * 2 * M_PI;
    Vector d = {cosf(phi) * sin_theta, sinf(phi) * sin_theta, sample.x};
    return to_world(&d, ez);
}

Vector square_to_uniform_sphere(Vector2f sample) {
    float phi = 2 * M_PI * sample.x, z = 1 - 2 * sample.y, r = sqrtf(1 - z * z);
    return (Vector) {r * cosf(phi), r * sinf(phi), z};
}

Vector square_to_uniform_cone(Vector2f sample, float cos_theta_max, const Vector *ez) {
    float cos_theta = (1 - sample.x) + sample.x * cos_theta_max;
    float sin_theta = sqrtf(fmaxf(0.0f, 1 - cos_theta * cos_theta));
    float phi = sample.y * 2 * M_PI;
    Vector d = {cosf(phi) * sin_theta, sinf(phi) * sin_theta, cos_theta};
    return to_world(&d, ez);
}

float fresnel(float costheta1, float n1, float n2) {
    if (n1 == n2)
        return 0.0;
    float eta = n1 / n2;
    float sintheta2 = eta * eta * (1 - costheta1 * costheta1);
    if (sintheta2 > 1.0) {
        // must reflect
        return 1.0;
    }
    float costheta2 = sqrtf(1 - sintheta2);
    float Rs = (n1 * costheta1 - n2 * costheta2)
               / (n1 * costheta1 + n2 * costheta2);
    float Rp = (n1 * costheta2 - n2 * costheta1)
               / (n1 * costheta2 + n2 * costheta1);
    return 0.5 * (Rs * Rs + Rp * Rp);
}
