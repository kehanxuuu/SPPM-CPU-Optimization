#include "intersection.h"
#include "warping.h"

Vector bsdf_sample(struct Intersection *isect, Vector2f sample) {
    switch (isect->hit->material) {
        case DIFFUSE:
            return bsdf_sample_diffuse(isect, sample);
        case SPECULAR:
            return bsdf_sample_specular(isect);
        case DIELECTRIC:
            return bsdf_sample_dielectic(isect, sample.x);
        default:
        UNIMPLEMENTED;
    }
}

Vector bsdf_eval(const struct Intersection *isect) {
    switch (isect->hit->material) {
        case DIFFUSE:
            return bsdf_eval_diffuse(isect);
        case SPECULAR:
            return bsdf_eval_specular(isect);
        case DIELECTRIC:
            return bsdf_eval_dielectic(isect);
        default:
        UNIMPLEMENTED;
    }
}

Vector bsdf_sample_diffuse(struct Intersection *isect, Vector2f sample) {
    if (vv_dot(&isect->wi, &isect->n) >= 0) {
        return ZERO_VEC;
    }
    isect->wo = square_to_cosine_hemisphere(sample, &isect->n);
    return isect->hit->albedo;
}

Vector bsdf_eval_diffuse(const struct Intersection *isect) {
    if (vv_dot(&isect->wi, &isect->n) >= 0 || vv_dot(&isect->wo, &isect->n) <= 0) {
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

Vector bsdf_sample_specular(struct Intersection *isect) {
    float cos_wi_N = -vv_dot(&isect->wi, &isect->n);
    Vector scaled_N = vs_mul(&isect->n, cos_wi_N);
    Vector scaled_N_2 = vs_mul(&scaled_N, 2);
    isect->wo = vv_add(&isect->wi, &scaled_N_2);
    return isect->hit->albedo; // (Vector) {1.0, 1.0, 1.0};
}

Vector bsdf_eval_specular(const struct Intersection *isect) {
    return ZERO_VEC;
}

float bsdf_pdf_specular(struct Intersection *isect) {
    return 0.0f;
}

Vector bsdf_sample_dielectic(struct Intersection *isect, float sample) {
    float costheta1 = -vv_dot(&isect->wi, &isect->n);
    float n1 = isect->interior ? isect->hit->ir : 1.0;
    float n2 = isect->interior ? 1.0 : isect->hit->ir;
    float fresnel_term = fresnel(costheta1, n1, n2);
    if (sample < fresnel_term) {
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

Vector bsdf_eval_dielectic(const struct Intersection *isect) {
    return ZERO_VEC;
}

float bsdf_pdf_dielectic(struct Intersection *isect) {
    return 0.0f;
}