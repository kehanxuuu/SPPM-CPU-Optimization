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

static inline void _mm256_masked_scatter_add_var_ps(__m256 x, __m256 y, __m256 z, __m256 mask, float* base_addr, int* index_loc){
    int mask_res = _mm256_movemask_ps(mask);
    if(mask_res == 0){
        return;
    }
    __m256 a = _mm256_set1_ps(1.0f);

    __m256 l01, u01, l23, u23;
    l01 = _mm256_unpacklo_ps(x, y);
    u01 = _mm256_unpackhi_ps(x, y);
    l23 = _mm256_unpacklo_ps(z, a);
    u23 = _mm256_unpackhi_ps(z, a);

    __m256 r04, r15, r26, r37;
    r04 = _mm256_shuffle_ps(l01, l23, 0b01000100);
    r15 = _mm256_shuffle_ps(u01, u23, 0b01000100);
    r26 = _mm256_shuffle_ps(l01, l23, 0b11101110);
    r37 = _mm256_shuffle_ps(u01, u23, 0b11101110);

    if(mask_res & 0b00000001){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[0]]);
        __m128 res = _mm_add_ps(inp, _mm256_castps256_ps128(r04));
        _mm_store_ps(&base_addr[4 * index_loc[0]], res);
    }
    if(mask_res & 0b00000010){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[1]]);
        __m128 res = _mm_add_ps(inp, _mm256_castps256_ps128(r15));
        _mm_store_ps(&base_addr[4 * index_loc[1]], res);
    }
    if(mask_res & 0b00000100){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[2]]);
        __m128 res = _mm_add_ps(inp, _mm256_castps256_ps128(r26));
        _mm_store_ps(&base_addr[4 * index_loc[2]], res);
    }
    if(mask_res & 0b00001000){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[3]]);
        __m128 res = _mm_add_ps(inp, _mm256_castps256_ps128(r37));
        _mm_store_ps(&base_addr[4 * index_loc[3]], res);
    }
    if(mask_res & 0b00010000){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[4]]);
        __m128 res = _mm_add_ps(inp, _mm256_extractf128_ps(r04, 1));
        _mm_store_ps(&base_addr[4 * index_loc[4]], res);
    }
    if(mask_res & 0b00100000){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[5]]);
        __m128 res = _mm_add_ps(inp, _mm256_extractf128_ps(r15, 1));
        _mm_store_ps(&base_addr[4 * index_loc[5]], res);
    }
    if(mask_res & 0b01000000){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[6]]);
        __m128 res = _mm_add_ps(inp, _mm256_extractf128_ps(r26, 1));
        _mm_store_ps(&base_addr[4 * index_loc[6]], res);
    }
    if(mask_res & 0b1000000){
        __m128 inp = _mm_load_ps(&base_addr[4 * index_loc[7]]);
        __m128 res = _mm_add_ps(inp, _mm256_extractf128_ps(r37, 1));
        _mm_store_ps(&base_addr[4 * index_loc[7]], res);
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

static inline void transpose8x8(float* line_0, float* line_1, float* line_2, float* line_3,
                                float* line_4, float* line_5, float* line_6, float* line_7,
                                __m256* res_0, __m256* res_1, __m256* res_2, __m256* res_3,
                                __m256* res_4, __m256* res_5, __m256* res_6, __m256* res_7){
    __m256  r0, r1, r2, r3, r4, r5, r6, r7;
    __m256  t0, t1, t2, t3, t4, t5, t6, t7;

    r0 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_0[0])), _mm_load_ps(&line_4[0]), 1);
    r1 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_1[0])), _mm_load_ps(&line_5[0]), 1);
    r2 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_2[0])), _mm_load_ps(&line_6[0]), 1);
    r3 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_3[0])), _mm_load_ps(&line_7[0]), 1);
    r4 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_0[4])), _mm_load_ps(&line_4[4]), 1);
    r5 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_1[4])), _mm_load_ps(&line_5[4]), 1);
    r6 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_2[4])), _mm_load_ps(&line_6[4]), 1);
    r7 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_3[4])), _mm_load_ps(&line_7[4]), 1);

    t0 = _mm256_unpacklo_ps(r0, r1);
    t1 = _mm256_unpackhi_ps(r0, r1);
    t2 = _mm256_unpacklo_ps(r2, r3);
    t3 = _mm256_unpackhi_ps(r2, r3);
    t4 = _mm256_unpacklo_ps(r4, r5);
    t5 = _mm256_unpackhi_ps(r4, r5);
    t6 = _mm256_unpacklo_ps(r6, r7);
    t7 = _mm256_unpackhi_ps(r6, r7);

    *res_0 = _mm256_shuffle_ps(t0, t2, 0x44);
    *res_1 = _mm256_shuffle_ps(t0, t2, 0xEE);
    *res_2 = _mm256_shuffle_ps(t1, t3, 0x44);
    *res_3 = _mm256_shuffle_ps(t1, t3, 0xEE);
    *res_4 = _mm256_shuffle_ps(t4, t6, 0x44);
    *res_5 = _mm256_shuffle_ps(t4, t6, 0xEE);
    *res_6 = _mm256_shuffle_ps(t5, t7, 0x44);
    *res_7 = _mm256_shuffle_ps(t5, t7, 0xEE);
}

static inline void transpose8x7(float* line_0, float* line_1, float* line_2, float* line_3,
                                float* line_4, float* line_5, float* line_6, float* line_7,
                                __m256* res_0, __m256* res_1, __m256* res_2, __m256* res_3,
                                __m256* res_4, __m256* res_5, __m256* res_6){
    __m256  r0, r1, r2, r3, r4, r5, r6, r7;
    __m256  t0, t1, t2, t3, t4, t5, t6, t7;

    r0 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_0[0])), _mm_load_ps(&line_4[0]), 1);
    r1 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_1[0])), _mm_load_ps(&line_5[0]), 1);
    r2 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_2[0])), _mm_load_ps(&line_6[0]), 1);
    r3 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_3[0])), _mm_load_ps(&line_7[0]), 1);
    r4 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_0[4])), _mm_load_ps(&line_4[4]), 1);
    r5 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_1[4])), _mm_load_ps(&line_5[4]), 1);
    r6 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_2[4])), _mm_load_ps(&line_6[4]), 1);
    r7 = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_load_ps(&line_3[4])), _mm_load_ps(&line_7[4]), 1);

    t0 = _mm256_unpacklo_ps(r0, r1);
    t1 = _mm256_unpackhi_ps(r0, r1);
    t2 = _mm256_unpacklo_ps(r2, r3);
    t3 = _mm256_unpackhi_ps(r2, r3);
    t4 = _mm256_unpacklo_ps(r4, r5);
    t5 = _mm256_unpackhi_ps(r4, r5);
    t6 = _mm256_unpacklo_ps(r6, r7);
    t7 = _mm256_unpackhi_ps(r6, r7);

    *res_0 = _mm256_shuffle_ps(t0, t2, 0x44);
    *res_1 = _mm256_shuffle_ps(t0, t2, 0xEE);
    *res_2 = _mm256_shuffle_ps(t1, t3, 0x44);
    *res_3 = _mm256_shuffle_ps(t1, t3, 0xEE);
    *res_4 = _mm256_shuffle_ps(t4, t6, 0x44);
    *res_5 = _mm256_shuffle_ps(t4, t6, 0xEE);
    *res_6 = _mm256_shuffle_ps(t5, t7, 0x44);
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
