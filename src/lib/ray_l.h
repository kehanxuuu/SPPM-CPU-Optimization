#ifndef TEAM32_RAY_L_H
#define TEAM32_RAY_L_H

#include <immintrin.h>

static inline void generate_ray8(const struct Camera *camera, __m256 px, __m256 py, __m256 sample0, __m256 sample1, __m256* o_x, __m256* o_y, __m256* o_z, __m256* d_x, __m256* d_y, __m256* d_z, __m256* t_max) {
    __m256 fov = _mm256_set1_ps(camera->fov);
    __m256 tg = _mm256_tanf_ps(_mm256_mul_ps(fov, _mm256_set1_ps(0.5f)));

    __m256 t0 = _mm256_mul_ps(_mm256_set1_ps(2.0f), _mm256_add_ps(px, sample0));
    __m256 t1 = _mm256_sub_ps(_mm256_div_ps(t0, _mm256_set1_ps(camera->W)), _mm256_set1_ps(1.0f));
    __m256 dc_x = _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(camera->aspect), tg), t1);

    __m256 t2 = _mm256_mul_ps(_mm256_set1_ps(2.0f), _mm256_add_ps(py, sample1));
    __m256 t3 = _mm256_sub_ps(_mm256_div_ps(t2, _mm256_set1_ps(camera->H)), _mm256_set1_ps(1.0f));
    __m256 dc_y = _mm256_mul_ps(tg, t3);

    __m256 dc_z = _mm256_set1_ps(1.0f);

    __m256 cur_d_x = _mm256_mul_ps(_mm256_set1_ps(camera->ex.x), dc_x);
    __m256 cur_d_y = _mm256_mul_ps(_mm256_set1_ps(camera->ex.y), dc_x);
    __m256 cur_d_z = _mm256_mul_ps(_mm256_set1_ps(camera->ex.z), dc_x);

    cur_d_x = _mm256_fmadd_ps(dc_y, _mm256_set1_ps(camera->ey.x), cur_d_x);
    cur_d_y = _mm256_fmadd_ps(dc_y, _mm256_set1_ps(camera->ey.y), cur_d_y);
    cur_d_z = _mm256_fmadd_ps(dc_y, _mm256_set1_ps(camera->ey.z), cur_d_z);

    cur_d_x = _mm256_fmadd_ps(dc_z, _mm256_set1_ps(camera->ez.x), cur_d_x);
    cur_d_y = _mm256_fmadd_ps(dc_z, _mm256_set1_ps(camera->ez.y), cur_d_y);
    cur_d_z = _mm256_fmadd_ps(dc_z, _mm256_set1_ps(camera->ez.z), cur_d_z);

    vector3fl_normalize(cur_d_x, cur_d_y, cur_d_z, d_x, d_y, d_z);
    *o_x = _mm256_set1_ps(camera->c.x);
    *o_y = _mm256_set1_ps(camera->c.y);
    *o_z = _mm256_set1_ps(camera->c.z);
    *t_max = _mm256_set1_ps(INFINITY);
}

#endif //TEAM32_RAY_L_H
