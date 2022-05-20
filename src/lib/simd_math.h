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

static inline __m256 _mm256_absf_ps(__m256 x){
    return _mm256_andnot_ps(_mm256_set1_ps(-0.0f), x);
}

static inline __m256 _mm256_neg_ps(__m256 x){
    return _mm256_sub_ps(_mm256_set1_ps(0.0f), x);
}
#endif //TEAM32_SIMD_MATH_H
