#ifndef TEAM32_SIMD_WARPING_H
#define TEAM32_SIMD_WARPING_H

#include "common.h"
#include "simd_random.h"
#include "simd_math.h"
#include "dt_l.h"
#include <immintrin.h>

void vector3fl_to_world(__m256 l_x, __m256 l_y, __m256 l_z, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z);

void vector3fl_square_to_cosine_hemisphere(__m256 sample0, __m256 sample1, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z);

void vector3fl_square_to_uniform_hemisphere(__m256 sample0, __m256 sample1, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z);

void vector3fl_square_to_uniform_sphere(__m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z);

void vector3fl_square_to_uniform_cone(__m256 sample0, __m256 sample1, __m256 cos_theta_max, __m256 normal_x, __m256 normal_y, __m256 normal_z, __m256* res_x, __m256* res_y, __m256* res_z);

__m256 vector3fl_fresnel(__m256 costheta1, __m256 n1, __m256 n2);
#endif //TEAM32_SIMD_WARPING_H
