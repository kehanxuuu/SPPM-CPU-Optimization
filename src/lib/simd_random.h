#ifndef TEAM32_SIMD_RANDOM_C_H
#define TEAM32_SIMD_RANDOM_C_H

#include <immintrin.h>
#include <stdint.h>

#define SIMD_RAND_MAX 0xffffffff

typedef struct{
    uint32_t cur_state;
} SimdRandomState;

extern SimdRandomState simd_random_state;

static inline uint32_t simd_rand(){
    uint32_t x = simd_random_state.cur_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    simd_random_state.cur_state = x;
    return simd_random_state.cur_state;
}

static inline void simd_seed(int seed){
    simd_random_state.cur_state = seed;
}

#endif //TEAM32_SIMD_RANDOM_C_H
