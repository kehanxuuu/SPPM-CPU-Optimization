#include "intersection.h"
#include "warping.h"

Vector bsdf_sample_diffuse(struct Intersection *isect, Vector2f sample) {
    if (vv_dot(&isect->wi, &isect->n) >= 0) {
        return ZERO_VEC;
    }
    isect->wo = square_to_cosine_hemisphere(sample, &isect->n);
//    isect->wo = square_to_uniform_sphere(sample);
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

Vector bsdf_sample_dielectic(struct Intersection *isect, Vector2f sample) {
    float costheta1 = -vv_dot(&isect->wi, &isect->n);
    float n1 = isect->interior? isect->hit->ir : 1.0;
    float n2 = isect->interior? 1.0 : isect->hit->ir;
    float fresnel_term = fresnel(costheta1, n1, n2);
    if (sample.x < fresnel_term) {
        // reflect
        Vector scaled_N = vs_mul(&isect->n, costheta1);
        Vector scaled_N_2 = vs_mul(&scaled_N, 2);
        isect->wo = vv_add(&isect->wi, &scaled_N_2);
    }
    else {
        // refract
        float eta = n1 / n2;
        float sintheta2 = eta * eta * (1.0 - costheta1 * costheta1);
        float tantheta2 = sqrtf(1.0 / (1.0 - sintheta2 * sintheta2) - 1.0);
        Vector scaled_N = vs_mul(&isect->n, costheta1);
        Vector perpendicular_N = vv_add(&isect->wi, &scaled_N);
        v_normalize(&perpendicular_N);
        vs_muleq(&perpendicular_N, tantheta2);
        Vector wo = vv_sub(&perpendicular_N, &isect->n);
        isect->wo = v_normalized(&wo);
    }
    return isect->hit->albedo;
}

Vector bsdf_eval_dielectic(struct Intersection *isect) {
    return ZERO_VEC;
}

float bsdf_pdf_dielectic(struct Intersection *isect) {
    return 0.0f;
}