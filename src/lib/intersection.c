#include "intersection.h"

Vector bsdf_sample_diffuse(struct Intersection *isect, Vector2f sample) {
    if (vv_dot(&isect->wi, &isect->n) >= 0) {
        return ZERO_VEC;
    }
    isect->wo = square_to_cosine_hemisphere(&sample, &isect->n);
    return isect->hit->albedo;
}

Vector bsdf_eval_diffuse(struct Intersection *isect) {
    if (vv_dot(&isect->wi, &isect->n) >= 0 || vv_dot(&isect->wo, &isect->n) <= 0){
        return ZERO_VEC;
    }
    return vs_mul(&isect->hit->albedo, INV_PI);
}

float bsdf_pdf_diffuse(struct Intersection *isect) {
    if (vv_dot(&isect->wi, &isect->n) >= 0 || vv_dot(&isect->wo, &isect->n) <= 0) {
        return 0.0f;
    }
    return vv_dot(&isect->wo, &isect->n) * INV_PI;
}

Vector bsdf_sample_specular(struct Intersection *isect, Vector2f sample) {
    float cos_wi_N = -vv_dot(&isect->wi, &isect->n);
    Vector scaled_N = vs_mul(&isect->n, cos_wi_N);
    Vector scaled_N_2 = vs_mul(&scaled_N, 2);
    isect->wo = vv_add(&isect->wi, &scaled_N_2);
    return isect->hit->albedo; // (Vector) {1.0, 1.0, 1.0};
}

Vector bsdf_eval_specular(struct Intersection *isect) {
    return ZERO_VEC;
}

float bsdf_pdf_specular(struct Intersection *isect) {
    return 0.0f;
}