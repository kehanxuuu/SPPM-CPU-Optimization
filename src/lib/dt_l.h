#ifndef TEAM32_DT_L_H
#define TEAM32_DT_L_H

#include "common.h"
#include "simd_random.h"
#include "simd_math.h"
#include <immintrin.h>

typedef struct {
    float* x;
    float* y;
    float* z;
} Vector3fL;

typedef struct {
    float* data;
} FloatL;

typedef struct {
    int* data;
} IntL;

typedef struct {
    void** data;
} PtrL;

typedef Vector3fL VectorL;

static inline void* malloc_align(size_t bytes){
//    bytes + alignment padding + end padding
    void* cur_res = malloc(bytes + ALIGN + ALIGN - bytes % ALIGN);
    return cur_res + ALIGN - (uint64_t)cur_res % ALIGN;
}

static inline void vector3fl_init(Vector3fL* vecl, size_t size){
    vecl->x = malloc_align(sizeof(float) * size);
    vecl->y = malloc_align(sizeof(float) * size);
    vecl->z = malloc_align(sizeof(float) * size);
}

static inline void floatl_init(FloatL* fl, size_t size){
    fl->data = malloc_align(sizeof(float) * size);
}

static inline void ptrl_init(PtrL* ptrl, size_t size){
    ptrl->data = malloc_align(sizeof(void*) * size);
}

static inline void intl_init(IntL* il, size_t size){
    il->data = malloc_align(sizeof(int) * size);
}

static inline void vector3fl_clear(Vector3fL* vecl, size_t size){
    memset(vecl->x, 0, sizeof(float) * size);
    memset(vecl->y, 0, sizeof(float) * size);
    memset(vecl->z, 0, sizeof(float) * size);
}

static inline void floatl_clear(FloatL* fl, size_t size){
    memset(fl->data, 0,sizeof(float) * size);
}

static inline void intl_clear(IntL* il, size_t size){
    memset(il->data, 0,sizeof(float) * size);
}

static inline void vector3fl_free(Vector3fL* vecl){
    free(vecl->x);
    free(vecl->y);
    free(vecl->z);
}

static inline void intl_free(IntL* il){
    free(il->data);
}

static inline void floatl_free(FloatL* fl){
    free(fl->data);
}

static inline void ptrl_free(PtrL* ptrl){
    free(ptrl->data);
}

static inline __m256 vector3fl_not_is_zero(__m256 x, __m256 y, __m256 z){
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 cmp_x = _mm256_cmp_ps(x, zero, _CMP_NEQ_OQ);
    __m256 cmp_y = _mm256_cmp_ps(y, zero, _CMP_NEQ_OQ);
    __m256 cmp_z = _mm256_cmp_ps(z, zero, _CMP_NEQ_OQ);

    __m256 or_xy = _mm256_or_ps(cmp_x, cmp_y);
    __m256 or_xyz = _mm256_or_ps(or_xy, cmp_z);
    return or_xyz;
}

static inline __m256 vector3fl_is_zero(__m256 x, __m256 y, __m256 z){
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 cmp_x = _mm256_cmp_ps(x, zero, _CMP_EQ_OQ);
    __m256 cmp_y = _mm256_cmp_ps(y, zero, _CMP_EQ_OQ);
    __m256 cmp_z = _mm256_cmp_ps(z, zero, _CMP_EQ_OQ);

    __m256 and_xy = _mm256_and_ps(cmp_x, cmp_y);
    __m256 and_xyz = _mm256_and_ps(and_xy, cmp_z);
    return and_xyz;
}

static inline void vector3fl_normalize(__m256 x, __m256 y, __m256 z, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 norm2 = _mm256_add_ps(_mm256_mul_ps(x, x), _mm256_add_ps(_mm256_mul_ps(y, y), _mm256_mul_ps(z, z)));
    __m256 inv_norm = _mm256_rsqrt_ps(norm2);
    *res_x = _mm256_mul_ps(x, inv_norm);
    *res_y = _mm256_mul_ps(y, inv_norm);
    *res_z = _mm256_mul_ps(y, inv_norm);
}

static inline void vector3fl_cross(__m256 ax, __m256 ay, __m256 az, __m256 bx, __m256 by, __m256 bz, __m256* res_x, __m256* res_y, __m256* res_z) {
    *res_x = _mm256_sub_ps(_mm256_mul_ps(ay, bz), _mm256_mul_ps(az, by));
    *res_y = _mm256_sub_ps(_mm256_mul_ps(az, bx), _mm256_mul_ps(ax, bz));
    *res_z = _mm256_sub_ps(_mm256_mul_ps(ax, by), _mm256_mul_ps(ay, bx));
}

static inline __m256 vector3fl_dot(__m256 ax, __m256 ay, __m256 az, __m256 bx, __m256 by, __m256 bz) {
    __m256 p0, p1, p2;
    p0 = _mm256_mul_ps(ax, bx);
    p1 = _mm256_mul_ps(ay, by);
    p2 = _mm256_mul_ps(az, bz);
    return _mm256_add_ps(_mm256_add_ps(p0, p1), p2);
}

#endif //TEAM32_DT_L_H
