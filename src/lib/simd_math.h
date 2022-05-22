#ifndef TEAM32_SIMD_MATH_H
#define TEAM32_SIMD_MATH_H

#include <immintrin.h>

// intel fuckery
// https://stackoverflow.com/questions/40475140/mathematical-functions-for-simd-registers

__m256 _ZGVdN8v_cosf(__m256 x);               /* _mm256_cos_ps(x)                         */
__m256 _ZGVdN8v_expf(__m256 x);               /* _mm256_exp_ps(x)                         */
__m256 _ZGVdN8v_logf(__m256 x);               /* _mm256_log_ps(x)                         */
__m256 _ZGVdN8v_sinf(__m256 x);               /* _mm256_sin_ps(x)                         */
__m256 _ZGVdN8vv_powf(__m256 x, __m256 y);    /* _mm256_pow_ps(x, y)                      */

static inline __m256 _mm256_cosf_ps(__m256 x){
    return _ZGVdN8v_cosf(x);
}

static inline __m256 _mm256_expf_ps(__m256 x){
    return _ZGVdN8v_expf(x);
}

static inline __m256 _mm256_logf_ps(__m256 x){
    return _ZGVdN8v_logf(x);
}

static inline __m256 _mm256_sinf_ps(__m256 x){
    return _ZGVdN8v_sinf(x);
}

static inline __m256 _mm256_powf_ps(__m256 x, __m256 y){
    return _ZGVdN8vv_powf(x, y);
}

static inline __m256 _mm256_tanf_ps(__m256 x){
    return _mm256_div_ps(_mm256_sinf_ps(x), _mm256_cosf_ps(x));
}

static inline __m256 _mm256_absf_ps(__m256 x){
    return _mm256_andnot_ps(_mm256_set1_ps(-0.0f), x);
}

static inline __m256 _mm256_neg_ps(__m256 x){
    return _mm256_sub_ps(_mm256_setzero_ps(), x);
}

static inline void _mm256_add_scatter_1_ps(__m256 x, float* base_addr, __m256i vindex){
    float x_impl[8];
    _mm256_store_ps(x_impl, x);
    int vindex_impl[8];
    _mm256_store_si256((__m256i *)vindex_impl, vindex);
    base_addr[vindex_impl[0]] += x_impl[0];
    base_addr[vindex_impl[1]] += x_impl[1];
    base_addr[vindex_impl[2]] += x_impl[2];
    base_addr[vindex_impl[3]] += x_impl[3];
    base_addr[vindex_impl[4]] += x_impl[4];
    base_addr[vindex_impl[5]] += x_impl[5];
    base_addr[vindex_impl[6]] += x_impl[6];
    base_addr[vindex_impl[7]] += x_impl[7];
}

static inline void _mm256_add_scatter_1_epi32(__m256i x, int* base_addr, __m256i vindex){
    int x_impl[8];
    _mm256_store_si256((__m256i *)x_impl, x);
    int vindex_impl[8];
    _mm256_store_si256((__m256i *)vindex_impl, vindex);
    base_addr[vindex_impl[0]] += x_impl[0];
    base_addr[vindex_impl[1]] += x_impl[1];
    base_addr[vindex_impl[2]] += x_impl[2];
    base_addr[vindex_impl[3]] += x_impl[3];
    base_addr[vindex_impl[4]] += x_impl[4];
    base_addr[vindex_impl[5]] += x_impl[5];
    base_addr[vindex_impl[6]] += x_impl[6];
    base_addr[vindex_impl[7]] += x_impl[7];
}

static inline __m256i _mm256_mod_epi32(__m256i x, __m256i y){
    return _mm256_set1_epi32(0.0f);
}
#endif //TEAM32_SIMD_MATH_H
