#include "simd_warping.h"

void vector3fl_to_world(__m256 l_x, __m256 l_y, __m256 l_z, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 cmp0 = _mm256_cmp_ps(_mm256_absf_ps(l_y), _mm256_set1_ps(0.9f), _CMP_LT_OQ);
    __m256 p_cmp_x = _mm256_blendv_ps(normal_y, _mm256_neg_ps( normal_z), cmp0);
    __m256 p_cmp_y = _mm256_blendv_ps(_mm256_neg_ps( normal_z), _mm256_set1_ps(0.0f), cmp0);
    __m256 p_cmp_z = _mm256_blendv_ps(_mm256_set1_ps(0.0f), normal_x, cmp0);

    __m256 n_x, n_y, n_z;
    vector3fl_normalize(p_cmp_x, p_cmp_y, p_cmp_z, &n_x, &n_y, &n_z);
    __m256 cross_x, cross_y, cross_z;
    vector3fl_cross(normal_x, normal_y, normal_z, n_x, n_y, n_z, &cross_x, &cross_y, &cross_z);

    __m256 accum_x, accum_y, accum_z;
    accum_x = _mm256_mul_ps(n_x, l_x);
    accum_x = _mm256_fmadd_ps(cross_x, l_y, accum_x);
    *res_x = _mm256_fmadd_ps(normal_x, l_z, accum_x);

    accum_y = _mm256_mul_ps(n_y, l_x);
    accum_y = _mm256_fmadd_ps(cross_y, l_y, accum_y);
    *res_y = _mm256_fmadd_ps(normal_y, l_z, accum_y);

    accum_z = _mm256_mul_ps(n_z, l_x);
    accum_z = _mm256_fmadd_ps(cross_z, l_y, accum_z);
    *res_z = _mm256_fmadd_ps(normal_z, l_z, accum_z);
}

void vector3fl_square_to_cosine_hemisphere(__m256 sample0, __m256 sample1, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 phi = _mm256_mul_ps(_mm256_set1_ps(M_2PI), sample0);
    __m256 r = _mm256_sqrt_ps( sample1);

    __m256 l_x, l_y, l_z;
    l_x = _mm256_mul_ps(r, _mm256_cosf_ps(phi));
    l_y = _mm256_mul_ps(r, _mm256_sinf_ps(phi));
    __m256 t0 = _mm256_sub_ps(_mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(l_x, l_x)), _mm256_mul_ps(l_y, l_y));
    l_z = _mm256_sqrt_ps(_mm256_max_ps(_mm256_set1_ps(0.0f), t0));

    vector3fl_to_world(l_x, l_y, l_z, normal_x, normal_y, normal_z, res_x, res_y, res_z);
}

void vector3fl_square_to_uniform_hemisphere(__m256 sample0, __m256 sample1, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 phi = _mm256_mul_ps(_mm256_set1_ps(M_2PI), sample0);
    __m256 t0 = _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(sample1, sample1));
    __m256 r = _mm256_sqrt_ps(_mm256_max_ps(_mm256_set1_ps(0.0f), t0));

    __m256 d_x = _mm256_mul_ps(r, _mm256_cosf_ps(phi));
    __m256 d_y = _mm256_mul_ps(r, _mm256_sinf_ps(phi));
    vector3fl_to_world(d_x, d_y, sample1, normal_x, normal_y, normal_z, res_x, res_y, res_z);
}

void vector3fl_square_to_uniform_sphere(__m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z) {
    __m256 phi = _mm256_mul_ps(_mm256_set1_ps(M_2PI), sample0);
    __m256 z = _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(_mm256_set1_ps(2.0f), sample1));
    __m256 r = _mm256_sqrt_ps(_mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(z, z)));

    *res_x = _mm256_mul_ps(r, _mm256_cosf_ps(phi));
    *res_y = _mm256_mul_ps(r, _mm256_sinf_ps(phi));
    *res_z = z;
}

void vector3fl_square_to_uniform_cone(__m256 sample0, __m256 sample1, __m256 cos_theta_max, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z) {
    __m256 cos_theta = _mm256_fmadd_ps(sample0, cos_theta_max, _mm256_sub_ps(_mm256_set1_ps(1.0f), sample0));
    __m256 t0 = _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(cos_theta, cos_theta));
    __m256 sin_theta = _mm256_sqrt_ps(_mm256_max_ps(_mm256_set1_ps(0.0f), t0));
    __m256 phi = _mm256_mul_ps(_mm256_set1_ps(M_2PI), sample1);

    __m256 d_x = _mm256_mul_ps(sin_theta, _mm256_cosf_ps(phi));
    __m256 d_y = _mm256_mul_ps(sin_theta, _mm256_sinf_ps(phi));
    vector3fl_to_world(d_x, d_y, cos_theta, normal_x, normal_y, normal_z, res_x, res_y, res_z);
}

__m256 vector3fl_fresnel(__m256 costheta1, __m256 n1, __m256 n2){
    __m256 res = _mm256_set1_ps(0.0f);
    __m256 cmp0 = _mm256_cmp_ps(n1, n2, _CMP_EQ_OQ);
    __m256 eta = _mm256_div_ps(n1, n2);
    __m256 sintheta2 = _mm256_mul_ps(_mm256_mul_ps(eta, eta), _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(costheta1, costheta1)));

    __m256 cmp1 = _mm256_cmp_ps(sintheta2, _mm256_set1_ps(1.0f), _CMP_GT_OQ);
    __m256 cmp01 = _mm256_andnot_ps(cmp0, cmp1);
    res = _mm256_blendv_ps(res, _mm256_set1_ps(1.0f), cmp01);

    __m256 costheta2 = _mm256_sqrt_ps(_mm256_sub_ps(_mm256_set1_ps(1.0f), sintheta2));
    __m256 n1_costheta1 = _mm256_mul_ps(n1, costheta1);
    __m256 n2_costheta2 = _mm256_mul_ps(n2, costheta2);
    __m256 Rs = _mm256_div_ps(_mm256_sub_ps(n1_costheta1, n2_costheta2), _mm256_add_ps(n1_costheta1, n2_costheta2));
    __m256 n1_costheta2 = _mm256_mul_ps(n1, costheta2);
    __m256 n2_costheta1 = _mm256_mul_ps(n2, costheta1);
    __m256 Rp = _mm256_div_ps(_mm256_sub_ps(n1_costheta2, n2_costheta1), _mm256_add_ps(n1_costheta2, n2_costheta1));
    __m256 t0 = _mm256_mul_ps(_mm256_set1_ps(0.5f), _mm256_add_ps(_mm256_mul_ps(Rs, Rs), _mm256_mul_ps(Rp, Rp)));
    res = _mm256_blendv_ps(t0, res, _mm256_or_ps(cmp0, cmp01));
    return res;
}