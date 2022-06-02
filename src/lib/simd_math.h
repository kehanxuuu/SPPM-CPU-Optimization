#ifndef TEAM32_SIMD_MATH_H
#define TEAM32_SIMD_MATH_H

#include <immintrin.h>
#include "avx_mathfun.h"

// intel fuckery
// https://stackoverflow.com/questions/40475140/mathematical-functions-for-simd-registers

__m256 _ZGVdN8v_cosf(__m256 x);               /* _mm256_cos_ps(x)                         */
__m256 _ZGVdN8v_expf(__m256 x);               /* _mm256_exp_ps(x)                         */
__m256 _ZGVdN8v_logf(__m256 x);               /* _mm256_log_ps(x)                         */
__m256 _ZGVdN8v_sinf(__m256 x);               /* _mm256_sin_ps(x)                         */
__m256 _ZGVdN8vv_powf(__m256 x, __m256 y);    /* _mm256_pow_ps(x, y)                      */

static inline __m256 _mm256_cosf_ps(__m256 x){
#if !defined(__APPLE__)
    return _ZGVdN8v_cosf(x);
#else
    return cos256_ps(x);
#endif
}

static inline __m256 _mm256_expf_ps(__m256 x){
#if !defined(__APPLE__)
    return _ZGVdN8v_expf(x);
#else
    return exp256_ps(x);
#endif
}

static inline __m256 _mm256_logf_ps(__m256 x){
#if !defined(__APPLE__)
    return _ZGVdN8v_logf(x);
#else
    return log256_ps(x);
#endif
}

static inline __m256 _mm256_sinf_ps(__m256 x){
#if !defined(__APPLE__)
    return _ZGVdN8v_sinf(x);
#else
    return sin256_ps(x);
#endif
}

static inline __m256 _mm256_powf_ps(__m256 x, __m256 y){
#if !defined(__APPLE__)
    return _ZGVdN8vv_powf(x, y);
#else
    return _mm256_setzero_si256(); // powf never called in sppm
#endif
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
    float x_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_ps(x_impl, x);
    int vindex_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_si256((__m256i *)vindex_impl, vindex);
    if(x_impl[0]) base_addr[vindex_impl[0]] += x_impl[0];
    if(x_impl[1]) base_addr[vindex_impl[1]] += x_impl[1];
    if(x_impl[2]) base_addr[vindex_impl[2]] += x_impl[2];
    if(x_impl[3]) base_addr[vindex_impl[3]] += x_impl[3];
    if(x_impl[4]) base_addr[vindex_impl[4]] += x_impl[4];
    if(x_impl[5]) base_addr[vindex_impl[5]] += x_impl[5];
    if(x_impl[6]) base_addr[vindex_impl[6]] += x_impl[6];
    if(x_impl[7]) base_addr[vindex_impl[7]] += x_impl[7];
}

static inline void _mm256_add_scatter_1_epi32(__m256i x, int* base_addr, __m256i vindex){
    int x_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_si256((__m256i *)x_impl, x);
    int vindex_impl[8] __attribute__((__aligned__(64)));
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

static inline void _mm256_masked_scatter_add_var_ps(__m256 x, __m256 y, __m256 z, __m256 mask, float* base_addr, __m256i vindex){
    int mask_res = _mm256_movemask_ps(mask);
    float x_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_ps(x_impl, x);
    float y_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_ps(y_impl, y);
    float z_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_ps(z_impl, z);
    int vindex_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_si256((__m256i *)vindex_impl, _mm256_mullo_epi32(vindex, _mm256_set1_epi32(4)));

    if(mask_res & 0b00000001){
        base_addr[vindex_impl[0] + 0] += x_impl[0];
        base_addr[vindex_impl[0] + 1] += y_impl[0];
        base_addr[vindex_impl[0] + 2] += z_impl[0];
        base_addr[vindex_impl[0] + 3] += 1;
    }
    if(mask_res & 0b00000010){
        base_addr[vindex_impl[1] + 0] += x_impl[1];
        base_addr[vindex_impl[1] + 1] += y_impl[1];
        base_addr[vindex_impl[1] + 2] += z_impl[1];
        base_addr[vindex_impl[1] + 3] += 1;
    }
    if(mask_res & 0b00000100){
        base_addr[vindex_impl[2] + 0] += x_impl[2];
        base_addr[vindex_impl[2] + 1] += y_impl[2];
        base_addr[vindex_impl[2] + 2] += z_impl[2];
        base_addr[vindex_impl[2] + 3] += 1;
    }
    if(mask_res & 0b00001000){
        base_addr[vindex_impl[3] + 0] += x_impl[3];
        base_addr[vindex_impl[3] + 1] += y_impl[3];
        base_addr[vindex_impl[3] + 2] += z_impl[3];
        base_addr[vindex_impl[3] + 3] += 1;
    }
    if(mask_res & 0b00010000){
        base_addr[vindex_impl[4] + 0] += x_impl[4];
        base_addr[vindex_impl[4] + 1] += y_impl[4];
        base_addr[vindex_impl[4] + 2] += z_impl[4];
        base_addr[vindex_impl[4] + 3] += 1;
    }
    if(mask_res & 0b00100000){
        base_addr[vindex_impl[5] + 0] += x_impl[5];
        base_addr[vindex_impl[5] + 1] += y_impl[5];
        base_addr[vindex_impl[5] + 2] += z_impl[5];
        base_addr[vindex_impl[5] + 3] += 1;
    }
    if(mask_res & 0b01000000){
        base_addr[vindex_impl[6] + 0] += x_impl[6];
        base_addr[vindex_impl[6] + 1] += y_impl[6];
        base_addr[vindex_impl[6] + 2] += z_impl[6];
        base_addr[vindex_impl[6] + 3] += 1;
    }
    if(mask_res & 0b1000000){
        base_addr[vindex_impl[7] + 0] += x_impl[7];
        base_addr[vindex_impl[7] + 1] += y_impl[7];
        base_addr[vindex_impl[7] + 2] += z_impl[7];
        base_addr[vindex_impl[7] + 3] += 1;
    }
}

static inline void _mm256_float4_descatter_ps(float* base_addr, int index, __m256* res_x, __m256* res_y, __m256* res_z, __m256* res_a){
    __m256i vindex = _mm256_setr_epi32(4 * index, 4 * (index + 1), 4 * (index + 2), 4 * (index + 3), 4 * (index + 4), 4 * (index + 5 ), 4 * (index + 6), 4 * (index + 7));
    *res_x = _mm256_i32gather_ps(base_addr, vindex, sizeof(float));
    vindex = _mm256_add_epi32(vindex, _mm256_set1_epi32(1));
    *res_y = _mm256_i32gather_ps(base_addr, vindex, sizeof(float));
    vindex = _mm256_add_epi32(vindex, _mm256_set1_epi32(1));
    *res_z = _mm256_i32gather_ps(base_addr, vindex, sizeof(float));
    vindex = _mm256_add_epi32(vindex, _mm256_set1_epi32(1));
    *res_a = _mm256_i32gather_ps(base_addr, vindex, sizeof(float));
}

static inline __m256i _mm256_mod_epi32(__m256i x, __m256i y){
    __m256 x_f = _mm256_cvtepi32_ps(x);
    __m256 y_f = _mm256_cvtepi32_ps(y);
    __m256 quotient = _mm256_div_ps(x_f, y_f);
    quotient = _mm256_floor_ps(quotient);
    __m256 remainder = _mm256_sub_ps(x_f, _mm256_mul_ps(quotient, y_f));
    return _mm256_cvtps_epi32(remainder);
}
#endif //TEAM32_SIMD_MATH_H
