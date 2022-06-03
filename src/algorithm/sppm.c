#include "sppm.h"
#include <time.h>
#include <stdlib.h>

void sppm_init(SPPM *sppm, int num_iterations, int ray_max_depth, int photon_num_iter, float initial_radius, Scene *scene, Camera *camera,
               Vector background) {
    sppm->num_iterations = num_iterations;
    sppm->ray_max_depth = ray_max_depth;
    sppm->num_photons = photon_num_iter;
    sppm->initial_radius = initial_radius;
    sppm->alpha = 2.0f / 3.0f;
    sppm->background = background;
    sppm->scene = scene;
    sppm->camera = camera;
}

void sppm_pixel_data_init(PixelData *pixel_datas, int size) {
    pixel_datas->size = size;
    pixel_datas->size_float_simd = (size / NUM_FLOAT_SIMD) * NUM_FLOAT_SIMD;
    floatl_init(&pixel_datas->radius, size);
    floatl_init(&pixel_datas->num_photons, size);
    vector3fl_init(&pixel_datas->tau, size);
    vector3fl_init(&pixel_datas->direct_radiance, size);
    floatl_init(&pixel_datas->cur_photons, size);
    vector3fl_init(&pixel_datas->cur_flux, size);
    vector3fl_init(&pixel_datas->cur_vp_attenuation, size);
    intersection_l_init(&pixel_datas->cur_vp_intersection, size);

    // initialize some with zero
    floatl_clear(&pixel_datas->num_photons, size);
    vector3fl_clear(&pixel_datas->tau, size);
    vector3fl_clear(&pixel_datas->direct_radiance, size);
    floatl_clear(&pixel_datas->cur_photons, size);
    vector3fl_clear(&pixel_datas->cur_flux, size);
}

void sppm_pixel_data_lookup_init(PixelDataLookup *lookup, int init_size) {
    lookup->fixed_size = init_size;
    lookup->hash_table = malloc(sizeof(IntArray) * init_size);
    for(int i = 0; i < init_size; i++) {
        arr_init_int(&lookup->hash_table[i], 20, 0);
    }
}

void sppm_pixel_data_lookup_assign(PixelDataLookup *lookup, float grid_size, Vector* grid_min){
    lookup->inv_grid_res = 1 / grid_size;
    lookup->grid_min = *grid_min;
}

void sppm_pixel_data_lookup_clear(PixelDataLookup *lookup) {
    for (int i = 0; i < lookup->fixed_size; i++) {
        lookup->hash_table[i].size = 0;
    }
}

void sppm_pixel_data_lookup_free(PixelDataLookup *lookup) {
    for (int i = 0; i < lookup->fixed_size; i++) {
        arr_free_int(&lookup->hash_table[i]);
    }
    free(lookup->hash_table);
}

void sppm_pixel_data_free(PixelData *pixel_datas) {
    floatl_free(&pixel_datas->radius);
    floatl_free(&pixel_datas->num_photons);
    vector3fl_free(&pixel_datas->tau);
    vector3fl_free(&pixel_datas->direct_radiance);
    floatl_free(&pixel_datas->cur_photons);
    vector3fl_free(&pixel_datas->cur_flux);
    vector3fl_free(&pixel_datas->cur_vp_attenuation);
    intersection_l_free(&pixel_datas->cur_vp_intersection);
}

int sppm_pixel_data_lookup_hash(PixelDataLookup *lookup, int x, int y, int z) {
    return ((x * 18397) + (y * 20483) + (z * 29303)) % lookup->fixed_size;
}

__m256i sppm_pixel_data_lookup_hash_l(PixelDataLookup *lookup, __m256i x, __m256i y, __m256i z) {
    // necessary for precision problem of SIMD multiplication
    __m256i x_m = _mm256_mullo_epi32(x, _mm256_set1_epi32(18397));
    __m256i y_m = _mm256_mullo_epi32(y, _mm256_set1_epi32(20483));
    __m256i z_m = _mm256_mullo_epi32(z, _mm256_set1_epi32(29303));
    return _mm256_mod_epi32(_mm256_add_epi32(_mm256_add_epi32(x_m, y_m), z_m), _mm256_set1_epi32(lookup->fixed_size));
}

void sppm_pixel_data_lookup_to_grid(PixelDataLookup *lookup, float loc_x, float loc_y, float loc_z, int* res_x, int *res_y, int* res_z) {
    *res_x = (int) fmaxf(0, (loc_x - lookup->grid_min.x) * lookup->inv_grid_res);
    *res_y = (int) fmaxf(0, (loc_y - lookup->grid_min.y) * lookup->inv_grid_res);
    *res_z = (int) fmaxf(0, (loc_z - lookup->grid_min.z) * lookup->inv_grid_res);
}

void sppm_pixel_data_lookup_to_grid_l(PixelDataLookup *lookup, __m256 loc_x, __m256 loc_y, __m256 loc_z, __m256* ind_x, __m256* ind_y, __m256* ind_z) {
    __m256 inv_grid_res = _mm256_set1_ps(lookup->inv_grid_res);
    __m256 grid_min_x = _mm256_set1_ps(lookup->grid_min.x);
    __m256 from_min_x = _mm256_sub_ps(loc_x, grid_min_x);
    *ind_x = _mm256_floor_ps(_mm256_max_ps(_mm256_setzero_ps(), _mm256_mul_ps(from_min_x, inv_grid_res)));

    __m256 grid_min_y = _mm256_set1_ps(lookup->grid_min.y);
    __m256 from_min_y = _mm256_sub_ps(loc_y, grid_min_y);
    *ind_y = _mm256_floor_ps(_mm256_max_ps(_mm256_setzero_ps(), _mm256_mul_ps(from_min_y, inv_grid_res)));

    __m256 grid_min_z = _mm256_set1_ps(lookup->grid_min.z);
    __m256 from_min_z = _mm256_sub_ps(loc_z, grid_min_z);
    *ind_z = _mm256_floor_ps(_mm256_max_ps(_mm256_setzero_ps(), _mm256_mul_ps(from_min_z, inv_grid_res)));
}

void sppm_pixel_data_lookup_store(PixelDataLookup *lookup, int loc_x, int loc_y, int loc_z, int pd_index) {
    int ht_loc = sppm_pixel_data_lookup_hash(lookup, loc_x, loc_y, loc_z);
    arr_add_int(&lookup->hash_table[ht_loc], &pd_index);
}

void sppm_build_pixel_data_lookup(PixelDataLookup *lookup, PixelData *pixel_datas) {
    __m256 grid_min_x = _mm256_set1_ps(FLT_MAX);
    __m256 grid_min_y = _mm256_set1_ps(FLT_MAX);
    __m256 grid_min_z = _mm256_set1_ps(FLT_MAX);
    __m256 grid_max_x = _mm256_set1_ps(-FLT_MAX);
    __m256 grid_max_y = _mm256_set1_ps(-FLT_MAX);
    __m256 grid_max_z = _mm256_set1_ps(-FLT_MAX);
    __m256 max_radius_l = _mm256_set1_ps(-FLT_MAX);

    float branch_cache[pixel_datas->size];
    int i;
    for(i = 0; i < pixel_datas->size_float_simd; i += NUM_FLOAT_SIMD){
        __m256 attenuation_x = _mm256_load_ps(&pixel_datas->cur_vp_attenuation.x[i]);
        __m256 attenuation_y = _mm256_load_ps(&pixel_datas->cur_vp_attenuation.y[i]);
        __m256 attenuation_z = _mm256_load_ps(&pixel_datas->cur_vp_attenuation.z[i]);

        __m256 attenuation_is_zero = vector3fl_is_zero(attenuation_x, attenuation_y, attenuation_z);
        _mm256_store_ps(&branch_cache[i], attenuation_is_zero);

        __m256 radius = _mm256_load_ps(&pixel_datas->radius.data[i]);
        max_radius_l = _mm256_max_ps(max_radius_l, radius);

        __m256 isect_p_x = _mm256_load_ps(&pixel_datas->cur_vp_intersection.p.x[i]);
        __m256 t0 = _mm256_sub_ps(isect_p_x, radius);
        __m256 t1 = _mm256_blendv_ps(t0, grid_min_x, attenuation_is_zero);
        grid_min_x = _mm256_min_ps(grid_min_x, t1);
        __m256 t2 = _mm256_add_ps(isect_p_x, radius);
        __m256 t3 = _mm256_blendv_ps(t2, grid_max_x, attenuation_is_zero);
        grid_max_x = _mm256_max_ps(grid_max_x, t3);

        __m256 isect_p_y = _mm256_load_ps(&pixel_datas->cur_vp_intersection.p.y[i]);
        __m256 t4 = _mm256_sub_ps(isect_p_y, radius);
        __m256 t5 = _mm256_blendv_ps(t4, grid_min_y, attenuation_is_zero);
        grid_min_y = _mm256_min_ps(grid_min_y, t5);
        __m256 t6 = _mm256_add_ps(isect_p_y, radius);
        __m256 t7 = _mm256_blendv_ps(t6, grid_max_y, attenuation_is_zero);
        grid_max_y = _mm256_max_ps(grid_max_y, t7);

        __m256 isect_p_z = _mm256_load_ps(&pixel_datas->cur_vp_intersection.p.z[i]);
        __m256 t8 = _mm256_sub_ps(isect_p_z, radius);
        __m256 t9 = _mm256_blendv_ps(t8, grid_min_z, attenuation_is_zero);
        grid_min_z = _mm256_min_ps(grid_min_z, t9);
        __m256 t10 = _mm256_add_ps(isect_p_z, radius);
        __m256 t11 = _mm256_blendv_ps(t10, grid_max_z, attenuation_is_zero);
        grid_max_z = _mm256_max_ps(grid_max_z, t11);
    }

    Vector grid_min, grid_max;
    float max_radius;
    float store_impl[NUM_FLOAT_SIMD] __attribute__((__aligned__(64)));
    _mm256_store_ps(store_impl, grid_min_x);
    grid_min.x = fminf(fminf(fminf(store_impl[0], store_impl[1]), fminf(store_impl[2], store_impl[3])),
                       fminf(fminf(store_impl[4], store_impl[5]), fminf(store_impl[6], store_impl[7])));
    _mm256_store_ps(store_impl, grid_min_y);
    grid_min.y = fminf(fminf(fminf(store_impl[0], store_impl[1]), fminf(store_impl[2], store_impl[3])),
                       fminf(fminf(store_impl[4], store_impl[5]), fminf(store_impl[6], store_impl[7])));
    _mm256_store_ps(store_impl, grid_min_z);
    grid_min.z = fminf(fminf(fminf(store_impl[0], store_impl[1]), fminf(store_impl[2], store_impl[3])),
                       fminf(fminf(store_impl[4], store_impl[5]), fminf(store_impl[6], store_impl[7])));

    _mm256_store_ps(store_impl, grid_max_x);
    grid_max.x = fmaxf(fmaxf(fmaxf(store_impl[0], store_impl[1]), fmaxf(store_impl[2], store_impl[3])),
                       fmaxf(fmaxf(store_impl[4], store_impl[5]), fmaxf(store_impl[6], store_impl[7])));
    _mm256_store_ps(store_impl, grid_max_y);
    grid_max.y = fmaxf(fmaxf(fmaxf(store_impl[0], store_impl[1]), fmaxf(store_impl[2], store_impl[3])),
                       fmaxf(fmaxf(store_impl[4], store_impl[5]), fmaxf(store_impl[6], store_impl[7])));
    _mm256_store_ps(store_impl, grid_max_z);
    grid_max.z = fmaxf(fmaxf(fmaxf(store_impl[0], store_impl[1]), fmaxf(store_impl[2], store_impl[3])),
                       fmaxf(fmaxf(store_impl[4], store_impl[5]), fmaxf(store_impl[6], store_impl[7])));

    _mm256_store_ps(store_impl, max_radius_l);
    max_radius = fmaxf(fmaxf(fmaxf(store_impl[0], store_impl[1]), fmaxf(store_impl[2], store_impl[3])),
                       fmaxf(fmaxf(store_impl[4], store_impl[5]), fmaxf(store_impl[6], store_impl[7])));

    for(; i < pixel_datas->size; i++){
        float radius_f = pixel_datas->radius.data[i];
        float attenuation_x = pixel_datas->cur_vp_attenuation.x[i];
        float attenuation_y = pixel_datas->cur_vp_attenuation.y[i];
        float attenuation_z = pixel_datas->cur_vp_attenuation.z[i];

        if (attenuation_x == 0 && attenuation_y == 0 && attenuation_z == 0) {
            branch_cache[i] = 1;
        }else{
            branch_cache[i] = 0;
            float pos_x = pixel_datas->cur_vp_intersection.p.x[i];
            float pos_y = pixel_datas->cur_vp_intersection.p.y[i];
            float pos_z = pixel_datas->cur_vp_intersection.p.z[i];

            grid_min.x = fminf(grid_min.x, pos_x - radius_f);
            grid_min.y = fminf(grid_min.y, pos_y - radius_f);
            grid_min.z = fminf(grid_min.z, pos_z - radius_f);

            grid_max.x = fminf(grid_max.x, pos_x + radius_f);
            grid_max.y = fminf(grid_max.y, pos_y + radius_f);
            grid_max.z = fminf(grid_max.z, pos_z + radius_f);

            max_radius = fmaxf(max_radius, radius_f);
        }
    }

    sppm_pixel_data_lookup_clear(lookup);
    sppm_pixel_data_lookup_assign(lookup, _SPPM_RADIUS_MULT * max_radius, &grid_min);

    for(i = 0; i < pixel_datas->size_float_simd; i += NUM_FLOAT_SIMD) {
        __m256 radius_f = _mm256_load_ps(&pixel_datas->radius.data[i]);

        __m256 cur_grid_min_x, cur_grid_min_y, cur_grid_min_z, cur_grid_max_x, cur_grid_max_y, cur_grid_max_z;
        __m256 isect_p_x = _mm256_load_ps(&pixel_datas->cur_vp_intersection.p.x[i]);
        cur_grid_min_x = _mm256_sub_ps(isect_p_x, radius_f);
        cur_grid_max_x = _mm256_add_ps(isect_p_x, radius_f);

        __m256 isect_p_y = _mm256_load_ps(&pixel_datas->cur_vp_intersection.p.y[i]);
        cur_grid_min_y = _mm256_sub_ps(isect_p_y, radius_f);
        cur_grid_max_y = _mm256_add_ps(isect_p_y, radius_f);

        __m256 isect_p_z = _mm256_load_ps(&pixel_datas->cur_vp_intersection.p.z[i]);
        cur_grid_min_z = _mm256_sub_ps(isect_p_z, radius_f);
        cur_grid_max_z = _mm256_add_ps(isect_p_z, radius_f);

        __m256 start_x, start_y, start_z, end_x, end_y, end_z;
        sppm_pixel_data_lookup_to_grid_l(lookup, cur_grid_min_x, cur_grid_min_y, cur_grid_min_z, &start_x, &start_y, &start_z);
        sppm_pixel_data_lookup_to_grid_l(lookup, cur_grid_max_x, cur_grid_max_y, cur_grid_max_z, &end_x, &end_y, &end_z);

        __m256 impossible_value = _mm256_set1_ps(-1.0f);
        __m256 branch_cache_result = _mm256_load_ps(&branch_cache[i]);
        start_x = _mm256_blendv_ps(start_x, impossible_value, branch_cache_result);
        start_y = _mm256_blendv_ps(start_y, impossible_value, branch_cache_result);
        start_z = _mm256_blendv_ps(start_z, impossible_value, branch_cache_result);
        end_x = _mm256_blendv_ps(end_x, impossible_value, branch_cache_result);
        end_y = _mm256_blendv_ps(end_y, impossible_value, branch_cache_result);
        end_z = _mm256_blendv_ps(end_z, impossible_value, branch_cache_result);

        int start_x_impl[8] __attribute__((__aligned__(64)));
        int start_y_impl[8] __attribute__((__aligned__(64)));
        int start_z_impl[8] __attribute__((__aligned__(64)));
        int end_x_impl[8] __attribute__((__aligned__(64)));
        int end_y_impl[8] __attribute__((__aligned__(64)));
        int end_z_impl[8] __attribute__((__aligned__(64)));

        _mm256_store_si256((__m256i *)start_x_impl, _mm256_cvtps_epi32(start_x));
        _mm256_store_si256((__m256i *)start_y_impl, _mm256_cvtps_epi32(start_y));
        _mm256_store_si256((__m256i *)start_z_impl, _mm256_cvtps_epi32(start_z));
        _mm256_store_si256((__m256i *)end_x_impl, _mm256_cvtps_epi32(end_x));
        _mm256_store_si256((__m256i *)end_y_impl, _mm256_cvtps_epi32(end_y));
        _mm256_store_si256((__m256i *)end_z_impl, _mm256_cvtps_epi32(end_z));

//      the lengths are (likely) too different, so it's inefficient to batch process
//      won't execute if branch_cache = 1
        for(int j = 0; j < NUM_FLOAT_SIMD; j++) {
            for (int x = start_x_impl[j] > 0 ? start_x_impl[j]: 0; x <= end_x_impl[j]; x++) {
                for (int y = start_y_impl[j] > 0 ? start_y_impl[j]: 0; y <= end_y_impl[j]; y++) {
                    for (int z = start_z_impl[j] > 0 ? start_z_impl[j]: 0; z <= end_z_impl[j]; z++) {
                        sppm_pixel_data_lookup_store(lookup, x, y, z, i+j);
                    }
                }
            }
        }
    }

    for(; i < pixel_datas->size; i++) {
        if(branch_cache[i]) {
            continue;
        }
        float radius_f = pixel_datas->radius.data[i];
        float p_x = pixel_datas->cur_vp_intersection.p.x[i];
        float p_y = pixel_datas->cur_vp_intersection.p.y[i];
        float p_z = pixel_datas->cur_vp_intersection.p.z[i];

        int start_x, start_y, start_z, end_x, end_y, end_z;
        sppm_pixel_data_lookup_to_grid(lookup, p_x - radius_f, p_y - radius_f, p_z - radius_f, &start_x, &start_y, &start_z);
        sppm_pixel_data_lookup_to_grid(lookup, p_x + radius_f, p_y + radius_f, p_z + radius_f, &end_x, &end_y, &end_z);

        for (int x = start_x; x <= end_x; x++) {
            for (int y = start_y; y <= end_y; y++) {
                for (int z = start_y; z <= end_z; z++) {
                    sppm_pixel_data_lookup_store(lookup, x, y, z, i);
                }
            }
        }
    }
}

void sppm_camera_pass(SPPM *sppm, PixelData *pixel_datas) {
    int W, H;
    W = sppm->camera->W;
    H = sppm->camera->H;

    __m256 x;
    __m256 y;
    int i;
    IntersectionM temp_isect;
    IntersectionM to_store_isect;
    sppm->ray_avg_depth = 0;
    for (i = 0; i < pixel_datas->size_float_simd; i += NUM_FLOAT_SIMD) {
        x = _mm256_load_ps(&sppm->launch_indices_x[i]);
        y = _mm256_load_ps(&sppm->launch_indices_y[i]);
        __m256 ray_o_x, ray_o_y, ray_o_z, ray_d_x, ray_d_y, ray_d_z, ray_t_max;
        __m256 samples0 = randf_full();
        __m256 samples1 = randf_full();
        generate_ray8(sppm->camera, x, y, samples0, samples1, &ray_o_x, &ray_o_y, &ray_o_z, &ray_d_x, &ray_d_y, &ray_d_z, &ray_t_max);

        __m256 attenuation_x = _mm256_set1_ps(1.0f);
        __m256 attenuation_y = _mm256_set1_ps(1.0f);
        __m256 attenuation_z = _mm256_set1_ps(1.0f);

        __m256 vp_attenuation_x = _mm256_setzero_ps();
        __m256 vp_attenuation_y = _mm256_setzero_ps();
        __m256 vp_attenuation_z = _mm256_setzero_ps();

        __m256 direct_radiance_x = _mm256_setzero_ps();
        __m256 direct_radiance_y = _mm256_setzero_ps();
        __m256 direct_radiance_z = _mm256_setzero_ps();

        __m256 not_completion_vector = _mm256_castsi256_ps(_mm256_set1_epi32(-1)); // highest bit must be 1 -> for _mm256_movemask_ps to work

        for (int c_depth = 0; c_depth < sppm->ray_max_depth; c_depth++) {
            sppm->ray_avg_depth += NUM_FLOAT_SIMD;
            __m256 do_intersect = scene_intersect_m(sppm->scene, ray_o_x, ray_o_y, ray_o_z, ray_d_x, ray_d_y, ray_d_z,
                                                    &ray_t_max, &temp_isect);

            __m256 no_isect_direct_radiance_x = _mm256_fmadd_ps(attenuation_x, _mm256_set1_ps(sppm->background.x),
                                                                direct_radiance_x);
            __m256 no_isect_direct_radiance_y = _mm256_fmadd_ps(attenuation_y, _mm256_set1_ps(sppm->background.y),
                                                                direct_radiance_y);
            __m256 no_isect_direct_radiance_z = _mm256_fmadd_ps(attenuation_z, _mm256_set1_ps(sppm->background.z),
                                                                direct_radiance_z);

            __m256 cur_selected = _mm256_andnot_ps(do_intersect, not_completion_vector);
            direct_radiance_x = _mm256_blendv_ps(direct_radiance_x, no_isect_direct_radiance_x, cur_selected);
            direct_radiance_y = _mm256_blendv_ps(direct_radiance_y, no_isect_direct_radiance_y, cur_selected);
            direct_radiance_z = _mm256_blendv_ps(direct_radiance_z, no_isect_direct_radiance_z, cur_selected);

            not_completion_vector = _mm256_blendv_ps(not_completion_vector, _mm256_setzero_ps(), cur_selected);

            __m256 emission_direct_radiance_x = _mm256_fmadd_ps(attenuation_x, temp_isect.mesh_emission.x,
                                                                direct_radiance_x);
            __m256 emission_direct_radiance_y = _mm256_fmadd_ps(attenuation_y, temp_isect.mesh_emission.y,
                                                                direct_radiance_y);
            __m256 emission_direct_radiance_z = _mm256_fmadd_ps(attenuation_z, temp_isect.mesh_emission.z,
                                                                direct_radiance_z);
            direct_radiance_x = _mm256_blendv_ps(direct_radiance_x, emission_direct_radiance_x, not_completion_vector);
            direct_radiance_y = _mm256_blendv_ps(direct_radiance_y, emission_direct_radiance_y, not_completion_vector);
            direct_radiance_z = _mm256_blendv_ps(direct_radiance_z, emission_direct_radiance_z, not_completion_vector);

            __m256 is_diffuse = _mm256_cmp_ps(temp_isect.mesh_material.data, _mm256_set1_ps(DIFFUSE), _CMP_EQ_OQ);

            cur_selected = _mm256_and_ps(is_diffuse, not_completion_vector);

            to_store_isect.mesh_material.data = _mm256_blendv_ps(to_store_isect.mesh_material.data,
                                                                 temp_isect.mesh_material.data, cur_selected);

            to_store_isect.mesh_albedo.x = _mm256_blendv_ps(to_store_isect.mesh_albedo.x, temp_isect.mesh_albedo.x,
                                                            cur_selected);
            to_store_isect.mesh_albedo.y = _mm256_blendv_ps(to_store_isect.mesh_albedo.y, temp_isect.mesh_albedo.y,
                                                            cur_selected);
            to_store_isect.mesh_albedo.z = _mm256_blendv_ps(to_store_isect.mesh_albedo.z, temp_isect.mesh_albedo.z,
                                                            cur_selected);
            to_store_isect.mesh_emission.x = _mm256_blendv_ps(to_store_isect.mesh_emission.x,
                                                              temp_isect.mesh_emission.x, cur_selected);
            to_store_isect.mesh_emission.y = _mm256_blendv_ps(to_store_isect.mesh_emission.y,
                                                              temp_isect.mesh_emission.y, cur_selected);
            to_store_isect.mesh_emission.z = _mm256_blendv_ps(to_store_isect.mesh_emission.z,
                                                              temp_isect.mesh_emission.z, cur_selected);
            to_store_isect.mesh_ir.data = _mm256_blendv_ps(to_store_isect.mesh_ir.data, temp_isect.mesh_ir.data,
                                                           cur_selected);
            to_store_isect.p.x = _mm256_blendv_ps(to_store_isect.p.x, temp_isect.p.x, cur_selected);
            to_store_isect.p.y = _mm256_blendv_ps(to_store_isect.p.y, temp_isect.p.y, cur_selected);
            to_store_isect.p.z = _mm256_blendv_ps(to_store_isect.p.z, temp_isect.p.z, cur_selected);
            to_store_isect.n.x = _mm256_blendv_ps(to_store_isect.n.x, temp_isect.n.x, cur_selected);
            to_store_isect.n.y = _mm256_blendv_ps(to_store_isect.n.y, temp_isect.n.y, cur_selected);
            to_store_isect.n.z = _mm256_blendv_ps(to_store_isect.n.z, temp_isect.n.z, cur_selected);
            to_store_isect.wi.x = _mm256_blendv_ps(to_store_isect.wi.x, temp_isect.wi.x, cur_selected);
            to_store_isect.wi.y = _mm256_blendv_ps(to_store_isect.wi.y, temp_isect.wi.y, cur_selected);
            to_store_isect.wi.z = _mm256_blendv_ps(to_store_isect.wi.z, temp_isect.wi.z, cur_selected);
            to_store_isect.interior.data = _mm256_blendv_ps(to_store_isect.interior.data, temp_isect.interior.data,
                                                            cur_selected);

            __m256 Ld_x, Ld_y, Ld_z;
            estimate_direct_lighting_m(sppm->scene, &temp_isect, &Ld_x, &Ld_y, &Ld_z);
            to_store_isect.wo.x = _mm256_blendv_ps(to_store_isect.wo.x, temp_isect.wo.x, cur_selected);
            to_store_isect.wo.y = _mm256_blendv_ps(to_store_isect.wo.y, temp_isect.wo.y, cur_selected);
            to_store_isect.wo.z = _mm256_blendv_ps(to_store_isect.wo.z, temp_isect.wo.z, cur_selected);
            vp_attenuation_x = _mm256_blendv_ps(vp_attenuation_x, attenuation_x, cur_selected);
            vp_attenuation_y = _mm256_blendv_ps(vp_attenuation_y, attenuation_y, cur_selected);
            vp_attenuation_z = _mm256_blendv_ps(vp_attenuation_z, attenuation_z, cur_selected);

            __m256 dl_direct_radiance_x = _mm256_fmadd_ps(attenuation_x, Ld_x, direct_radiance_x);
            __m256 dl_direct_radiance_y = _mm256_fmadd_ps(attenuation_y, Ld_y, direct_radiance_y);
            __m256 dl_direct_radiance_z = _mm256_fmadd_ps(attenuation_z, Ld_z, direct_radiance_z);
            direct_radiance_x = _mm256_blendv_ps(direct_radiance_x, dl_direct_radiance_x, cur_selected);
            direct_radiance_y = _mm256_blendv_ps(direct_radiance_y, dl_direct_radiance_y, cur_selected);
            direct_radiance_z = _mm256_blendv_ps(direct_radiance_z, dl_direct_radiance_z, cur_selected);

            not_completion_vector = _mm256_blendv_ps(not_completion_vector, _mm256_setzero_ps(), cur_selected);
            if (_mm256_movemask_ps(not_completion_vector) == 0) {  // all diffuse
                break;
            }

            __m256 cur_attenuation_x, cur_attenuation_y, cur_attenuation_z;
            __m256 mesh_material = temp_isect.mesh_material.data;
            __m256 is_dielectric = _mm256_cmp_ps(_mm256_set1_ps(DIELECTRIC), mesh_material, _CMP_EQ_OQ);

            int is_dielectric_int = _mm256_movemask_ps(is_dielectric);
            if (is_dielectric_int == 0xFF) {  // all dielectric
                __m256 samples2 = randf_full();
                bsdf_sample_dielectric_m(&temp_isect, samples2, &cur_attenuation_x, &cur_attenuation_y, &cur_attenuation_z);
            } else if (is_dielectric_int == 0) {  // all specular
                bsdf_sample_specular_m(&temp_isect, &cur_attenuation_x, &cur_attenuation_y, &cur_attenuation_z);
            } else {  // mixed
                __m256 samples2 = randf_full();
                __m256 specular_res_x, specular_res_y, specular_res_z;
                __m256 dielectric_res_x, dielectric_res_y, dielectric_res_z;
                bsdf_sample_specular_m(&temp_isect, &specular_res_x, &specular_res_y, &specular_res_z);
                __m256 spec_wo_x, spec_wo_y, spec_wo_z;
                spec_wo_x = temp_isect.wo.x;
                spec_wo_y = temp_isect.wo.y;
                spec_wo_z = temp_isect.wo.z;
                bsdf_sample_dielectric_m(&temp_isect, samples2, &dielectric_res_x, &dielectric_res_y, &dielectric_res_z);
                temp_isect.wo.x = _mm256_blendv_ps(spec_wo_x, temp_isect.wo.x, is_dielectric);
                temp_isect.wo.y = _mm256_blendv_ps(spec_wo_y, temp_isect.wo.y, is_dielectric);
                temp_isect.wo.z = _mm256_blendv_ps(spec_wo_z, temp_isect.wo.z, is_dielectric);
                cur_attenuation_x = _mm256_blendv_ps(specular_res_x, dielectric_res_x, is_dielectric);
                cur_attenuation_y = _mm256_blendv_ps(specular_res_y, dielectric_res_y, is_dielectric);
                cur_attenuation_z = _mm256_blendv_ps(specular_res_z, dielectric_res_z, is_dielectric);
            }

            to_store_isect.wo.x = _mm256_blendv_ps(to_store_isect.wo.x, temp_isect.wo.x, not_completion_vector);
            to_store_isect.wo.y = _mm256_blendv_ps(to_store_isect.wo.y, temp_isect.wo.y, not_completion_vector);
            to_store_isect.wo.z = _mm256_blendv_ps(to_store_isect.wo.z, temp_isect.wo.z, not_completion_vector);

            __m256 cur_attenuation_is_zero = vector3fl_is_zero(cur_attenuation_x, cur_attenuation_y, cur_attenuation_z);
            not_completion_vector = _mm256_blendv_ps(not_completion_vector, _mm256_setzero_ps(), cur_attenuation_is_zero);

            attenuation_x = _mm256_mul_ps(attenuation_x, cur_attenuation_x);
            attenuation_y = _mm256_mul_ps(attenuation_y, cur_attenuation_y);
            attenuation_z = _mm256_mul_ps(attenuation_z, cur_attenuation_z);

            __m256 continue_prob = vector3fl_cwise_max(attenuation_x, attenuation_y, attenuation_z);
            __m256 try_roulette = _mm256_cmp_ps(continue_prob, _mm256_set1_ps(0.25f), _CMP_LT_OQ);
            __m256 dead_roulette = _mm256_cmp_ps(randf_full(), continue_prob, _CMP_GE_OQ);
            cur_selected = _mm256_and_ps(_mm256_andnot_ps(dead_roulette, try_roulette), not_completion_vector);
            __m256 end_selected = _mm256_and_ps(_mm256_and_ps(dead_roulette, try_roulette), not_completion_vector);
            not_completion_vector = _mm256_blendv_ps(not_completion_vector, _mm256_setzero_ps(), end_selected);

            __m256 updated_attenuation_x = _mm256_div_ps(attenuation_x, continue_prob);
            __m256 updated_attenuation_y = _mm256_div_ps(attenuation_y, continue_prob);
            __m256 updated_attenuation_z = _mm256_div_ps(attenuation_z, continue_prob);

            attenuation_x = _mm256_blendv_ps(attenuation_x, updated_attenuation_x, cur_selected);
            attenuation_y = _mm256_blendv_ps(attenuation_y, updated_attenuation_y, cur_selected);
            attenuation_z = _mm256_blendv_ps(attenuation_z, updated_attenuation_z, cur_selected);

            __m256 epsilon = _mm256_set1_ps(EPSILON);
            ray_d_x = temp_isect.wo.x;
            ray_d_y = temp_isect.wo.y;
            ray_d_z = temp_isect.wo.z;
            ray_o_x = _mm256_fmadd_ps(ray_d_x, epsilon, temp_isect.p.x);
            ray_o_y = _mm256_fmadd_ps(ray_d_y, epsilon, temp_isect.p.y);
            ray_o_z = _mm256_fmadd_ps(ray_d_z, epsilon, temp_isect.p.z);
            ray_t_max = _mm256_set1_ps(INFINITY);

            if (_mm256_movemask_ps(not_completion_vector) == 0) {
                break;
            }
        }

        __m256 new_direct_radiance_x = _mm256_add_ps(direct_radiance_x, _mm256_load_ps(&pixel_datas->direct_radiance.x[i]));
        __m256 new_direct_radiance_y = _mm256_add_ps(direct_radiance_y, _mm256_load_ps(&pixel_datas->direct_radiance.y[i]));
        __m256 new_direct_radiance_z = _mm256_add_ps(direct_radiance_z, _mm256_load_ps(&pixel_datas->direct_radiance.z[i]));
        _mm256_store_ps(&pixel_datas->direct_radiance.x[i], new_direct_radiance_x);
        _mm256_store_ps(&pixel_datas->direct_radiance.y[i], new_direct_radiance_y);
        _mm256_store_ps(&pixel_datas->direct_radiance.z[i], new_direct_radiance_z);

        _mm256_store_ps(&pixel_datas->cur_vp_attenuation.x[i], vp_attenuation_x);
        _mm256_store_ps(&pixel_datas->cur_vp_attenuation.y[i], vp_attenuation_y);
        _mm256_store_ps(&pixel_datas->cur_vp_attenuation.z[i], vp_attenuation_z);

        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_material.data[i], to_store_isect.mesh_material.data);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_albedo.x[i], to_store_isect.mesh_albedo.x);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_albedo.y[i], to_store_isect.mesh_albedo.y);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_albedo.z[i], to_store_isect.mesh_albedo.z);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_emission.x[i], to_store_isect.mesh_emission.x);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_emission.y[i], to_store_isect.mesh_emission.y);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_emission.z[i], to_store_isect.mesh_emission.z);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.mesh_ir.data[i], to_store_isect.mesh_ir.data);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.p.x[i], to_store_isect.p.x);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.p.y[i], to_store_isect.p.y);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.p.z[i], to_store_isect.p.z);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.n.x[i], to_store_isect.n.x);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.n.y[i], to_store_isect.n.y);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.n.z[i], to_store_isect.n.z);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.wi.x[i], to_store_isect.wi.x);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.wi.y[i], to_store_isect.wi.y);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.wi.z[i], to_store_isect.wi.z);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.wo.x[i], to_store_isect.wo.x);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.wo.y[i], to_store_isect.wo.y);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.wo.z[i], to_store_isect.wo.z);
        _mm256_store_ps(&pixel_datas->cur_vp_intersection.interior.data[i], to_store_isect.interior.data);

    }

    int x_impl[8] __attribute__((__aligned__(64)));
    int y_impl[8] __attribute__((__aligned__(64)));
    _mm256_store_si256((__m256i *)x_impl, _mm256_cvtps_epi32(x));
    _mm256_store_si256((__m256i *)y_impl, _mm256_cvtps_epi32(y));
    // remaining paths
    for(int k = 0; i < pixel_datas->size; i++, k++){
        Ray ray = generate_ray(sppm->camera, x_impl[k], y_impl[k], (Vector2f) {randf(), randf()});
        Vector attenuation = {1.0f, 1.0f, 1.0f};
        Vector vp_attenuation = ZERO_VEC;
        Vector direct_radiance = ZERO_VEC;
        Intersection isect;
        for (int c_depth = 0; c_depth < sppm->ray_max_depth; c_depth++) {
            sppm->ray_avg_depth++;
            if (!scene_intersect(sppm->scene, &ray, &isect)) {
                vvv_fmaeq(&direct_radiance, &attenuation, &sppm->background);
                break;
            }
            vvv_fmaeq(&direct_radiance, &attenuation, &isect.hit->emission);
            if (isect.hit->material == DIFFUSE) {
                Vector Ld = estimate_direct_lighting(sppm->scene, &isect);
                vvv_fmaeq(&direct_radiance, &attenuation, &Ld);
                vp_attenuation = attenuation;
                break;
            }
            Vector cur_attenuation;
            switch (isect.hit->material) {
                case SPECULAR:
                    cur_attenuation = bsdf_sample_specular(&isect);
                    break;
                case DIELECTRIC:
                    cur_attenuation = bsdf_sample_dielectic(&isect, randf());
                    break;
                default:
                    UNIMPLEMENTED;
            }
            if (vv_equal(&cur_attenuation, &ZERO_VEC)) {
                break;
            }
            vv_muleq(&attenuation, &cur_attenuation);
            float continue_prob = v_cwise_max(&attenuation);
            if (continue_prob < 0.25) {
                if (randf() >= continue_prob)
                    break;
                vs_diveq(&attenuation, continue_prob);
            }
            ray = (Ray) {isect.p, isect.wo, INFINITY};
            ray.o = ray_at(&ray, EPSILON);
        }

        pixel_datas->direct_radiance.x[i] = direct_radiance.x;
        pixel_datas->direct_radiance.y[i] = direct_radiance.y;
        pixel_datas->direct_radiance.z[i] = direct_radiance.z;

        pixel_datas->cur_vp_attenuation.x[i] = vp_attenuation.x;
        pixel_datas->cur_vp_attenuation.y[i] = vp_attenuation.y;
        pixel_datas->cur_vp_attenuation.z[i] = vp_attenuation.z;

        pixel_datas->cur_vp_intersection.mesh_material.data[i] = isect.hit->material;
        pixel_datas->cur_vp_intersection.mesh_albedo.x[i] = isect.hit->albedo.x;
        pixel_datas->cur_vp_intersection.mesh_albedo.y[i] = isect.hit->albedo.y;
        pixel_datas->cur_vp_intersection.mesh_albedo.z[i] = isect.hit->albedo.z;
        pixel_datas->cur_vp_intersection.mesh_emission.x[i] = isect.hit->emission.x;
        pixel_datas->cur_vp_intersection.mesh_emission.y[i] = isect.hit->emission.y;
        pixel_datas->cur_vp_intersection.mesh_emission.z[i] = isect.hit->emission.z;
        pixel_datas->cur_vp_intersection.mesh_ir.data[i] = isect.hit->ir;
        pixel_datas->cur_vp_intersection.p.x[i] = isect.p.x;
        pixel_datas->cur_vp_intersection.p.y[i] = isect.p.y;
        pixel_datas->cur_vp_intersection.p.z[i] = isect.p.z;
        pixel_datas->cur_vp_intersection.n.x[i] = isect.n.x;
        pixel_datas->cur_vp_intersection.n.y[i] = isect.n.y;
        pixel_datas->cur_vp_intersection.n.z[i] = isect.n.z;
        pixel_datas->cur_vp_intersection.wi.x[i] = isect.wi.x;
        pixel_datas->cur_vp_intersection.wi.y[i] = isect.wi.y;
        pixel_datas->cur_vp_intersection.wi.z[i] = isect.wi.z;
        pixel_datas->cur_vp_intersection.wo.x[i] = isect.wo.x;
        pixel_datas->cur_vp_intersection.wo.y[i] = isect.wo.y;
        pixel_datas->cur_vp_intersection.wo.z[i] = isect.wo.z;
        pixel_datas->cur_vp_intersection.interior.data[i] = isect.interior;
    }
    sppm->ray_avg_depth /= (float) pixel_datas->size;
    fprintf(stderr, "\tray average depth: %f ", sppm->ray_avg_depth);
}

void sppm_photon_pass(SPPM *sppm, PixelDataLookup *lookup, PixelData *pixel_datas) {
    sppm->photon_avg_depth = 0;
    sppm->photon_avg_lookups = 0;
    int i;
    for (i = 0; i < (sppm->num_photons / NUM_FLOAT_SIMD) * NUM_FLOAT_SIMD; i += NUM_FLOAT_SIMD) {
        __m256 not_completion_vector = _mm256_castsi256_ps(_mm256_set1_epi32(-1));

        __m256 pdf_emitter = _mm256_set1_ps(sppm->scene->accum_probabilities[1]);
        __m256i emitter_id = _mm256_setzero_si256();
        __m256 sample = randf_full();
        for (int j = 0; j < sppm->scene->n_emitters; ++j) {
            __m256 ac_prob_i = _mm256_set1_ps(sppm->scene->accum_probabilities[j]);
            __m256 ac_prob_i1 = _mm256_set1_ps(sppm->scene->accum_probabilities[j + 1]);
            __m256 cmp0 = _mm256_and_ps(_mm256_cmp_ps(ac_prob_i, sample, _CMP_LE_OQ),
                                        _mm256_cmp_ps(sample, ac_prob_i1, _CMP_LT_OQ));
            pdf_emitter = _mm256_blendv_ps(pdf_emitter, _mm256_sub_ps(ac_prob_i1, ac_prob_i), cmp0);
            __m256i mask_i = (__m256i)(cmp0);
            emitter_id = _mm256_blendv_epi8(emitter_id, _mm256_set1_epi32(j), mask_i);
        }
//      only spheres
        int emitter_id_impl[NUM_FLOAT_SIMD] __attribute__((__aligned__(64)));
        _mm256_store_si256((__m256i *)emitter_id_impl, emitter_id);
        Sphere *spheres[NUM_FLOAT_SIMD] __attribute__((__aligned__(64)));
        for (int j = 0; j < NUM_FLOAT_SIMD; j++) {
            spheres[j] = sppm->scene->emitters[emitter_id_impl[j]]->geometry->data;
        }

        __m256 normal_x, normal_y, normal_z;
        vector3fl_square_to_uniform_sphere(randf_full(), randf_full(), &normal_x, &normal_y, &normal_z);
        __m256 sphere_c_x = _mm256_setr_ps(spheres[0]->c.x, spheres[1]->c.x, spheres[2]->c.x, spheres[3]->c.x,
                                           spheres[4]->c.x, spheres[5]->c.x, spheres[6]->c.x, spheres[7]->c.x);
        __m256 sphere_c_y = _mm256_setr_ps(spheres[0]->c.y, spheres[1]->c.y, spheres[2]->c.y, spheres[3]->c.y,
                                           spheres[4]->c.y, spheres[5]->c.y, spheres[6]->c.y, spheres[7]->c.y);
        __m256 sphere_c_z = _mm256_setr_ps(spheres[0]->c.z, spheres[1]->c.z, spheres[2]->c.z, spheres[3]->c.z,
                                           spheres[4]->c.z, spheres[5]->c.z, spheres[6]->c.z, spheres[7]->c.z);
        __m256 sphere_r = _mm256_setr_ps(spheres[0]->r, spheres[1]->r, spheres[2]->r, spheres[3]->r,
                                         spheres[4]->r, spheres[5]->r, spheres[6]->r, spheres[7]->r);
        __m256 mult_s = _mm256_add_ps(sphere_r, _mm256_set1_ps(EPSILON));
        __m256 ray_o_x = _mm256_fmadd_ps(normal_x, mult_s, sphere_c_x);
        __m256 ray_o_y = _mm256_fmadd_ps(normal_y, mult_s, sphere_c_y);
        __m256 ray_o_z = _mm256_fmadd_ps(normal_z, mult_s, sphere_c_z);

        __m256 One = _mm256_set1_ps(1.0f);
        __m256 pdf_pos = _mm256_div_ps(One,
                                       _mm256_mul_ps(_mm256_set1_ps(M_2PI), _mm256_mul_ps(sphere_r, sphere_r)));
        __m256 ray_d_x, ray_d_y, ray_d_z;
        vector3fl_square_to_uniform_hemisphere(randf_full(), randf_full(), normal_x, normal_y, normal_z, &ray_d_x,
                                               &ray_d_y, &ray_d_z);
        __m256 pdf_dir = _mm256_set1_ps(M_1_PI * 0.5);
        __m256 ray_t_max = _mm256_set1_ps(INFINITY);

        __m256 emission_x = _mm256_setr_ps(sppm->scene->emitters[emitter_id_impl[0]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[1]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[2]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[3]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[4]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[5]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[6]]->emission.x,
                                           sppm->scene->emitters[emitter_id_impl[7]]->emission.x);
        __m256 emission_y = _mm256_setr_ps(sppm->scene->emitters[emitter_id_impl[0]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[1]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[2]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[3]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[4]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[5]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[6]]->emission.y,
                                           sppm->scene->emitters[emitter_id_impl[7]]->emission.y);
        __m256 emission_z = _mm256_setr_ps(sppm->scene->emitters[emitter_id_impl[0]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[1]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[2]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[3]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[4]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[5]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[6]]->emission.z,
                                           sppm->scene->emitters[emitter_id_impl[7]]->emission.z);

        __m256 t0 = _mm256_div_ps(One, _mm256_mul_ps(pdf_emitter, _mm256_mul_ps(pdf_pos, pdf_dir)));
        __m256 light_radiance_x = _mm256_mul_ps(emission_x, t0);
        __m256 light_radiance_y = _mm256_mul_ps(emission_y, t0);
        __m256 light_radiance_z = _mm256_mul_ps(emission_z, t0);

        IntersectionM temp_isect;
        for (int c_depth = 0; c_depth < sppm->ray_max_depth; c_depth++) {
            sppm->photon_avg_depth += NUM_FLOAT_SIMD;
            __m256 do_intersect = scene_intersect_m(sppm->scene, ray_o_x, ray_o_y, ray_o_z, ray_d_x, ray_d_y, ray_d_z,
                                                    &ray_t_max, &temp_isect);
            not_completion_vector = _mm256_blendv_ps(not_completion_vector, _mm256_setzero_ps(),
                                                     _mm256_andnot_ps(do_intersect, not_completion_vector));

            if (c_depth > 0) {
                __m256 loc_x, loc_y, loc_z;
                sppm_pixel_data_lookup_to_grid_l(lookup, temp_isect.p.x, temp_isect.p.y, temp_isect.p.z, &loc_x, &loc_y,
                                                 &loc_z);
                __m256i ht_loc = sppm_pixel_data_lookup_hash_l(lookup, _mm256_cvtps_epi32(loc_x),
                                                               _mm256_cvtps_epi32(loc_y), _mm256_cvtps_epi32(loc_z));
                int ht_loc_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_si256((__m256i *)ht_loc_impl, ht_loc);
                float isect_p_x_impl[8] __attribute__((__aligned__(64)));
                float isect_p_y_impl[8] __attribute__((__aligned__(64)));
                float isect_p_z_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(isect_p_x_impl, temp_isect.p.x);
                _mm256_store_ps(isect_p_y_impl, temp_isect.p.y);
                _mm256_store_ps(isect_p_z_impl, temp_isect.p.z);
                int not_complete = _mm256_movemask_ps(not_completion_vector);

                float light_rad_x_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(light_rad_x_impl, light_radiance_x);
                float light_rad_y_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(light_rad_y_impl, light_radiance_y);
                float light_rad_z_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(light_rad_z_impl, light_radiance_z);

                float wo_x_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(wo_x_impl, _mm256_neg_ps(ray_d_x));
                float wo_y_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(wo_y_impl, _mm256_neg_ps(ray_d_y));
                float wo_z_impl[8] __attribute__((__aligned__(64)));
                _mm256_store_ps(wo_z_impl, _mm256_neg_ps(ray_d_z));

                // lookup table has varying length
                // instead of using SIMD to visit 8 tables simultaneously, visit each table sequentially with 8 indices at a time
                for (int j = 0; j < NUM_FLOAT_SIMD; j++) {
                    if ((not_complete & 0b1) == 0) {
                        not_complete >>= 1;
                        continue;
                    }
                    not_complete >>= 1;

                    __m256 isect_p_x = _mm256_set1_ps(isect_p_x_impl[j]);
                    __m256 isect_p_y = _mm256_set1_ps(isect_p_y_impl[j]);
                    __m256 isect_p_z = _mm256_set1_ps(isect_p_z_impl[j]);

                    __m256 wo_x = _mm256_set1_ps(wo_x_impl[j]);
                    __m256 wo_y = _mm256_set1_ps(wo_y_impl[j]);
                    __m256 wo_z = _mm256_set1_ps(wo_z_impl[j]);

                    __m256 light_rad_x = _mm256_set1_ps(light_rad_x_impl[j]);
                    __m256 light_rad_y = _mm256_set1_ps(light_rad_y_impl[j]);
                    __m256 light_rad_z = _mm256_set1_ps(light_rad_z_impl[j]);

                    int cur_arr_ind;
                    for (cur_arr_ind = 0; cur_arr_ind < (lookup->hash_table[ht_loc_impl[j]].size / NUM_FLOAT_SIMD) *
                                                            NUM_FLOAT_SIMD; cur_arr_ind += NUM_FLOAT_SIMD) {
                        sppm->photon_avg_lookups += NUM_FLOAT_SIMD;
                        __m256i pd_index = _mm256_load_si256((__m256i *)&lookup->hash_table[ht_loc_impl[j]].data[cur_arr_ind]);

                        __m256 radius = _mm256_i32gather_ps(pixel_datas->radius.data, pd_index, sizeof(float));
                        __m256 cur_vp_intersection_x = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.p.x,
                                                                           pd_index, sizeof(float));
                        __m256 cur_vp_intersection_y = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.p.y,
                                                                           pd_index, sizeof(float));
                        __m256 cur_vp_intersection_z = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.p.z,
                                                                           pd_index, sizeof(float));

                        __m256 dist_between_x = _mm256_sub_ps(cur_vp_intersection_x, isect_p_x);
                        __m256 dist_between_y = _mm256_sub_ps(cur_vp_intersection_y, isect_p_y);
                        __m256 dist_between_z = _mm256_sub_ps(cur_vp_intersection_z, isect_p_z);

                        __m256 sqr_dist_between = vector3fl_sqrnorm(dist_between_x, dist_between_y, dist_between_z);
                        __m256 cmp0 = _mm256_cmp_ps(sqr_dist_between, _mm256_mul_ps(radius, radius), _CMP_LT_OQ);

                        __m256 cur_vp_intersection_n_x = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.n.x,
                                                                             pd_index, sizeof(float));
                        __m256 cur_vp_intersection_n_y = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.n.y,
                                                                             pd_index, sizeof(float));
                        __m256 cur_vp_intersection_n_z = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.n.z,
                                                                             pd_index, sizeof(float));

                        __m256 cur_vp_intersection_wi_x = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.wi.x,
                                                                              pd_index, sizeof(float));
                        __m256 cur_vp_intersection_wi_y = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.wi.y,
                                                                              pd_index, sizeof(float));
                        __m256 cur_vp_intersection_wi_z = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.wi.z,
                                                                              pd_index, sizeof(float));

                        __m256 t1 = vector3fl_dot(cur_vp_intersection_n_x, cur_vp_intersection_n_y,
                                                  cur_vp_intersection_n_z, cur_vp_intersection_wi_x,
                                                  cur_vp_intersection_wi_y, cur_vp_intersection_wi_z);
                        __m256 t2 = vector3fl_dot(cur_vp_intersection_n_x, cur_vp_intersection_n_y,
                                                  cur_vp_intersection_n_z, wo_x, wo_y, wo_z);
                        __m256 cmp1 = _mm256_and_ps(_mm256_cmp_ps(t1, _mm256_setzero_ps(), _CMP_LT_OQ),
                                                    _mm256_cmp_ps(t2, _mm256_setzero_ps(), _CMP_GT_OQ));

                        __m256 cur_vp_intersection_mesh_albedo_x = _mm256_i32gather_ps(
                                pixel_datas->cur_vp_intersection.mesh_albedo.x, pd_index, sizeof(float));
                        __m256 cur_vp_intersection_mesh_albedo_y = _mm256_i32gather_ps(
                                pixel_datas->cur_vp_intersection.mesh_albedo.y, pd_index, sizeof(float));
                        __m256 cur_vp_intersection_mesh_albedo_z = _mm256_i32gather_ps(
                                pixel_datas->cur_vp_intersection.mesh_albedo.z, pd_index, sizeof(float));

                        __m256 inv_pi = _mm256_set1_ps(INV_PI);
                        __m256 bsdf_x = _mm256_blendv_ps(_mm256_setzero_ps(),
                                                         _mm256_mul_ps(cur_vp_intersection_mesh_albedo_x, inv_pi),
                                                         cmp1);
                        __m256 bsdf_y = _mm256_blendv_ps(_mm256_setzero_ps(),
                                                         _mm256_mul_ps(cur_vp_intersection_mesh_albedo_y, inv_pi),
                                                         cmp1);
                        __m256 bsdf_z = _mm256_blendv_ps(_mm256_setzero_ps(),
                                                         _mm256_mul_ps(cur_vp_intersection_mesh_albedo_z, inv_pi),
                                                         cmp1);

                        __m256 bsdf_x_times_rad = _mm256_mul_ps(bsdf_x, light_rad_x);
                        __m256 bsdf_y_times_rad = _mm256_mul_ps(bsdf_y, light_rad_y);
                        __m256 bsdf_z_times_rad = _mm256_mul_ps(bsdf_z, light_rad_z);

                        _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), bsdf_x_times_rad, cmp0), pixel_datas->cur_flux.x, pd_index);
                        _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), bsdf_y_times_rad, cmp0), pixel_datas->cur_flux.y, pd_index);
                        _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), bsdf_z_times_rad, cmp0), pixel_datas->cur_flux.z, pd_index);
                        _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), _mm256_set1_ps(1),  cmp0), pixel_datas->cur_photons.data, pd_index);
                    }

                    // remaining lookups
                    for (; cur_arr_ind < lookup->hash_table[ht_loc_impl[j]].size; cur_arr_ind++) {
                        sppm->photon_avg_lookups++;
                        int pd_index = arr_get_int(&lookup->hash_table[ht_loc_impl[j]], cur_arr_ind);
                        Vector cur_vp_attenuation = { pixel_datas->cur_vp_attenuation.x[pd_index],
                                               pixel_datas->cur_vp_attenuation.y[pd_index],
                                               pixel_datas->cur_vp_attenuation.z[pd_index] };
                        if (vv_equal(&cur_vp_attenuation, &ZERO_VEC))
                            continue;

                        Vector cur_vp_intersection_pos = { pixel_datas->cur_vp_intersection.p.x[pd_index],
                                                           pixel_datas->cur_vp_intersection.p.y[pd_index],
                                                           pixel_datas->cur_vp_intersection.p.z[pd_index] };
                        float radius = pixel_datas->radius.data[pd_index];
                        Vector isect_p = {isect_p_x_impl[j], isect_p_y_impl[j], isect_p_z_impl[j]};
                        Vector dist_between = vv_sub(&cur_vp_intersection_pos, &isect_p);
                        if (v_norm_sqr(&dist_between) < radius * radius) {
                            Vector cur_vp_intersection_wi = { pixel_datas->cur_vp_intersection.wi.x[pd_index],
                                                              pixel_datas->cur_vp_intersection.wi.y[pd_index],
                                                              pixel_datas->cur_vp_intersection.wi.z[pd_index] };
                            Vector cur_vp_intersection_n = { pixel_datas->cur_vp_intersection.n.x[pd_index],
                                                             pixel_datas->cur_vp_intersection.n.y[pd_index],
                                                             pixel_datas->cur_vp_intersection.n.z[pd_index] };
                            Vector cur_neg_ray_d = { -ray_d_x[j], -ray_d_y[j], -ray_d_z[j] };
                            Vector bsdf = ZERO_VEC;
                            if (vv_dot(&cur_vp_intersection_wi, &cur_vp_intersection_n) < 0 && vv_dot(&cur_neg_ray_d, &cur_vp_intersection_n) > 0) { //  && pixel_datas->cur_vp_intersection.mesh_material.data[pd_index] == DIFFUSE -> not necessary, as cur_vp_attenuation = 0
                                Vector cur_vp_intersection_albedo = { pixel_datas->cur_vp_intersection.mesh_albedo.x[pd_index],
                                                                      pixel_datas->cur_vp_intersection.mesh_albedo.y[pd_index],
                                                                      pixel_datas->cur_vp_intersection.mesh_albedo.z[pd_index] };
                                bsdf = vs_mul(&cur_vp_intersection_albedo, INV_PI);
                            }
                            Vector light_radiance = { light_rad_x_impl[j], light_rad_y_impl[j], light_rad_z_impl[j] };
                            vv_muleq(&bsdf, &light_radiance);
                            pixel_datas->cur_flux.x[pd_index] += bsdf.x;
                            pixel_datas->cur_flux.y[pd_index] += bsdf.y;
                            pixel_datas->cur_flux.z[pd_index] += bsdf.z;
                            pixel_datas->cur_photons.data[pd_index] += 1.0;
                        }
                    }
                }
            }

            __m256 cur_attenuation_x, cur_attenuation_y, cur_attenuation_z;
            bsdf_sample_m(&temp_isect, &cur_attenuation_x, &cur_attenuation_y, &cur_attenuation_z);

            light_radiance_x = _mm256_mul_ps(light_radiance_x, cur_attenuation_x);
            light_radiance_y = _mm256_mul_ps(light_radiance_y, cur_attenuation_y);
            light_radiance_z = _mm256_mul_ps(light_radiance_z, cur_attenuation_z);

            __m256 continue_prob = vector3fl_cwise_max(light_radiance_x, light_radiance_y, light_radiance_z);
            __m256 try_roulette = _mm256_cmp_ps(continue_prob, _mm256_set1_ps(0.25f), _CMP_LT_OQ);
            __m256 dead_roulette = _mm256_cmp_ps(randf_full(), continue_prob, _CMP_GE_OQ);
            __m256 cur_selected = _mm256_and_ps(_mm256_andnot_ps(dead_roulette, try_roulette),
                                                not_completion_vector);
            __m256 end_selected = _mm256_and_ps(_mm256_and_ps(dead_roulette, try_roulette), not_completion_vector);
            not_completion_vector = _mm256_blendv_ps(not_completion_vector, _mm256_setzero_ps(), end_selected);

            __m256 updated_attenuation_x = _mm256_div_ps(light_radiance_x, continue_prob);
            __m256 updated_attenuation_y = _mm256_div_ps(light_radiance_y, continue_prob);
            __m256 updated_attenuation_z = _mm256_div_ps(light_radiance_z, continue_prob);

            light_radiance_x = _mm256_blendv_ps(light_radiance_x, updated_attenuation_x, cur_selected);
            light_radiance_y = _mm256_blendv_ps(light_radiance_y, updated_attenuation_y, cur_selected);
            light_radiance_z = _mm256_blendv_ps(light_radiance_z, updated_attenuation_z, cur_selected);

            __m256 epsilon = _mm256_set1_ps(EPSILON);
            ray_d_x = temp_isect.wo.x;
            ray_d_y = temp_isect.wo.y;
            ray_d_z = temp_isect.wo.z;
            ray_o_x = _mm256_fmadd_ps(ray_d_x, epsilon, temp_isect.p.x);
            ray_o_y = _mm256_fmadd_ps(ray_d_y, epsilon, temp_isect.p.y);
            ray_o_z = _mm256_fmadd_ps(ray_d_z, epsilon, temp_isect.p.z);
            ray_t_max = _mm256_set1_ps(INFINITY);

            if (_mm256_movemask_ps(not_completion_vector) == 0) {
                break;
            }
        }
    }

    // remaining photons
    for (; i < sppm->num_photons; i++) {
        Ray ray;
        float pdf_emitter, pdf_pos, pdf_dir;
        Mesh *emitter = sample_emitter(sppm->scene, randf(), &pdf_emitter);
        ray = sphere_surface_photon_sample((Sphere *) emitter->geometry->data, (Vector2f) {randf(), randf()},
                                           (Vector2f) {randf(), randf()},
                                           &pdf_pos, &pdf_dir);

        Vector light_radiance = vs_div(&emitter->emission, pdf_emitter * pdf_pos * pdf_dir);
        for (int c_depth = 0; c_depth < sppm->ray_max_depth; c_depth++) {
            sppm->photon_avg_depth++;
            Intersection isect;
            if (!scene_intersect(sppm->scene, &ray, &isect)) {
                break;
            }

            if (c_depth > 0) {  // Direct illumination is accounted for in the camera pass
                int loc_x, loc_y, loc_z;
                sppm_pixel_data_lookup_to_grid(lookup, isect.p.x, isect.p.y, isect.p.z, &loc_x, &loc_y, &loc_z);

                __m256 isect_p_x = _mm256_set1_ps(isect.p.x);
                __m256 isect_p_y = _mm256_set1_ps(isect.p.y);
                __m256 isect_p_z = _mm256_set1_ps(isect.p.z);

                __m256 cur_neg_ray_d_x = _mm256_neg_ps(_mm256_set1_ps(ray.d.x));
                __m256 cur_neg_ray_d_y = _mm256_neg_ps(_mm256_set1_ps(ray.d.y));
                __m256 cur_neg_ray_d_z = _mm256_neg_ps(_mm256_set1_ps(ray.d.z));
                int ht_loc = sppm_pixel_data_lookup_hash(lookup, loc_x, loc_y, loc_z);
                int cur_arr_ind;
                for (cur_arr_ind = 0; cur_arr_ind < (lookup->hash_table[ht_loc].size / NUM_FLOAT_SIMD) *
                                                    NUM_FLOAT_SIMD; cur_arr_ind += NUM_FLOAT_SIMD) {
                    sppm->photon_avg_lookups += NUM_FLOAT_SIMD;
                    __m256i pd_index = _mm256_load_si256((__m256i *) &lookup->hash_table[ht_loc].data[cur_arr_ind]);
                    __m256 cur_vp_attenuation_x = _mm256_i32gather_ps(pixel_datas->cur_vp_attenuation.x, pd_index,
                                                                      sizeof(float));
                    __m256 cur_vp_attenuation_y = _mm256_i32gather_ps(pixel_datas->cur_vp_attenuation.y, pd_index,
                                                                      sizeof(float));
                    __m256 cur_vp_attenuation_z = _mm256_i32gather_ps(pixel_datas->cur_vp_attenuation.z, pd_index,
                                                                      sizeof(float));
                    __m256 cur_vp_attenuation_is_zero = vector3fl_is_zero(cur_vp_attenuation_x, cur_vp_attenuation_y,
                                                                          cur_vp_attenuation_z);

                    __m256 radius = _mm256_i32gather_ps(pixel_datas->radius.data, pd_index, sizeof(float));
                    __m256 cur_vp_intersection_x = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.p.x, pd_index,
                                                                       sizeof(float));
                    __m256 cur_vp_intersection_y = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.p.y, pd_index,
                                                                       sizeof(float));
                    __m256 cur_vp_intersection_z = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.p.z, pd_index,
                                                                       sizeof(float));

                    __m256 dist_between_x = _mm256_sub_ps(cur_vp_intersection_x, isect_p_x);
                    __m256 dist_between_y = _mm256_sub_ps(cur_vp_intersection_y, isect_p_y);
                    __m256 dist_between_z = _mm256_sub_ps(cur_vp_intersection_z, isect_p_z);

                    __m256 sqr_dist_between = vector3fl_sqrnorm(dist_between_x, dist_between_y, dist_between_z);
                    __m256 cmp0 = _mm256_cmp_ps(sqr_dist_between, _mm256_mul_ps(radius, radius), _CMP_LT_OQ);
                    cmp0 = _mm256_andnot_ps(cur_vp_attenuation_is_zero, cmp0);

                    __m256 cur_vp_intersection_n_x = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.n.x,
                                                                         pd_index, sizeof(float));
                    __m256 cur_vp_intersection_n_y = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.n.y,
                                                                         pd_index, sizeof(float));
                    __m256 cur_vp_intersection_n_z = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.n.z,
                                                                         pd_index, sizeof(float));

                    __m256 cur_vp_intersection_wi_x = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.wi.x,
                                                                          pd_index, sizeof(float));
                    __m256 cur_vp_intersection_wi_y = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.wi.y,
                                                                          pd_index, sizeof(float));
                    __m256 cur_vp_intersection_wi_z = _mm256_i32gather_ps(pixel_datas->cur_vp_intersection.wi.z,
                                                                          pd_index, sizeof(float));

                    __m256 t1 = vector3fl_dot(cur_vp_intersection_n_x, cur_vp_intersection_n_y,
                                              cur_vp_intersection_n_z, cur_vp_intersection_wi_x,
                                              cur_vp_intersection_wi_y, cur_vp_intersection_wi_z);
                    __m256 t2 = vector3fl_dot(cur_vp_intersection_n_x, cur_vp_intersection_n_y,
                                              cur_vp_intersection_n_z, cur_neg_ray_d_x, cur_neg_ray_d_y,
                                              cur_neg_ray_d_z);
                    __m256 cmp1 = _mm256_and_ps(_mm256_cmp_ps(t1, _mm256_setzero_ps(), _CMP_LT_OQ),
                                                _mm256_cmp_ps(t2, _mm256_setzero_ps(), _CMP_GT_OQ));

                    __m256 cur_vp_intersection_mesh_albedo_x = _mm256_i32gather_ps(
                            pixel_datas->cur_vp_intersection.mesh_albedo.x, pd_index, sizeof(float));
                    __m256 cur_vp_intersection_mesh_albedo_y = _mm256_i32gather_ps(
                            pixel_datas->cur_vp_intersection.mesh_albedo.y, pd_index, sizeof(float));
                    __m256 cur_vp_intersection_mesh_albedo_z = _mm256_i32gather_ps(
                            pixel_datas->cur_vp_intersection.mesh_albedo.z, pd_index, sizeof(float));

                    __m256 inv_pi = _mm256_set1_ps(INV_PI);
                    __m256 bsdf_x = _mm256_blendv_ps(_mm256_setzero_ps(),
                                                     _mm256_mul_ps(cur_vp_intersection_mesh_albedo_x, inv_pi),
                                                     cmp1);
                    __m256 bsdf_y = _mm256_blendv_ps(_mm256_setzero_ps(),
                                                     _mm256_mul_ps(cur_vp_intersection_mesh_albedo_y, inv_pi),
                                                     cmp1);
                    __m256 bsdf_z = _mm256_blendv_ps(_mm256_setzero_ps(),
                                                     _mm256_mul_ps(cur_vp_intersection_mesh_albedo_z, inv_pi),
                                                     cmp1);

                    bsdf_x = _mm256_mul_ps(bsdf_x, _mm256_set1_ps(light_radiance.x));
                    bsdf_y = _mm256_mul_ps(bsdf_y, _mm256_set1_ps(light_radiance.y));
                    bsdf_z = _mm256_mul_ps(bsdf_z, _mm256_set1_ps(light_radiance.z));

                    _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), bsdf_x, cmp0),
                                            pixel_datas->cur_flux.x, pd_index);
                    _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), bsdf_y, cmp0),
                                            pixel_datas->cur_flux.y, pd_index);
                    _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), bsdf_z, cmp0),
                                            pixel_datas->cur_flux.z, pd_index);
                    _mm256_add_scatter_1_ps(_mm256_blendv_ps(_mm256_setzero_ps(), _mm256_set1_ps(1), cmp0),
                                               pixel_datas->cur_photons.data,
                                               pd_index);
                }

                Vector isect_p = {isect.p.x, isect.p.y, isect.p.z};
                Vector light_radiance_vec = {light_radiance.x, light_radiance.y, light_radiance.z};
                for (; cur_arr_ind < lookup->hash_table[ht_loc].size; cur_arr_ind++) {
                    sppm->photon_avg_lookups++;
                    int pd_index = arr_get_int(&lookup->hash_table[ht_loc], cur_arr_ind);
                    Vector cur_vp_attenuation = {pixel_datas->cur_vp_attenuation.x[pd_index],
                                                 pixel_datas->cur_vp_attenuation.y[pd_index],
                                                 pixel_datas->cur_vp_attenuation.z[pd_index]};
                    if (vv_equal(&cur_vp_attenuation, &ZERO_VEC))
                        continue;

                    Vector cur_vp_intersection_pos = {pixel_datas->cur_vp_intersection.p.x[pd_index],
                                                      pixel_datas->cur_vp_intersection.p.y[pd_index],
                                                      pixel_datas->cur_vp_intersection.p.z[pd_index]};
                    float radius = pixel_datas->radius.data[pd_index];
                    Vector dist_between = vv_sub(&cur_vp_intersection_pos, &isect_p);
                    if (v_norm_sqr(&dist_between) < radius * radius) {
                        Vector cur_vp_intersection_wi = {pixel_datas->cur_vp_intersection.wi.x[pd_index],
                                                         pixel_datas->cur_vp_intersection.wi.y[pd_index],
                                                         pixel_datas->cur_vp_intersection.wi.z[pd_index]};
                        Vector cur_vp_intersection_n = {pixel_datas->cur_vp_intersection.n.x[pd_index],
                                                        pixel_datas->cur_vp_intersection.n.y[pd_index],
                                                        pixel_datas->cur_vp_intersection.n.z[pd_index]};
                        Vector cur_neg_ray_d = {-ray.d.x, -ray.d.y, -ray.d.z};
                        Vector bsdf = ZERO_VEC;
                        if (vv_dot(&cur_vp_intersection_wi, &cur_vp_intersection_n) < 0 &&
                            vv_dot(&cur_neg_ray_d, &cur_vp_intersection_n) > 0) {
                            Vector cur_vp_intersection_albedo = {
                                    pixel_datas->cur_vp_intersection.mesh_albedo.x[pd_index],
                                    pixel_datas->cur_vp_intersection.mesh_albedo.y[pd_index],
                                    pixel_datas->cur_vp_intersection.mesh_albedo.z[pd_index]};
                            bsdf = vs_mul(&cur_vp_intersection_albedo, INV_PI);
                        }
                        vv_muleq(&bsdf, &light_radiance_vec);
                        pixel_datas->cur_flux.x[pd_index] += bsdf.x;
                        pixel_datas->cur_flux.y[pd_index] += bsdf.y;
                        pixel_datas->cur_flux.z[pd_index] += bsdf.z;
                        pixel_datas->cur_photons.data[pd_index] += 1.0;
                    }
                }
            }
            Vector cur_attenuation = bsdf_sample(&isect, (Vector2f) {randf(), randf()});
            vv_muleq(&light_radiance, &cur_attenuation);
            float continue_prob = v_cwise_max(&light_radiance);
            // Russian Roulette
            if (continue_prob < 0.25) {
                if (randf() >= continue_prob) {
                    break;
                }
                vs_diveq(&light_radiance, continue_prob);
            }
            ray = (Ray) {isect.p, isect.wo, INFINITY};
            ray.o = ray_at(&ray, EPSILON);
        }
    }
    sppm->photon_avg_depth /= (float) sppm->num_photons;
    sppm->photon_avg_lookups /= (float) sppm->num_photons;
    fprintf(stderr, "\tphoton average depth: %f, hash table lookups: %f ", sppm->photon_avg_depth, sppm->photon_avg_lookups);
}

void sppm_consolidate(PixelData *pixel_datas, float alpha) {
    __m256 alpha_vector = _mm256_set1_ps(alpha);
    int i;
    for (i = 0; i < pixel_datas->size_float_simd; i += NUM_FLOAT_SIMD) {
        __m256 cur_vp_attenuation_x = _mm256_load_ps(&pixel_datas->cur_vp_attenuation.x[i]);
        __m256 cur_vp_attenuation_y = _mm256_load_ps(&pixel_datas->cur_vp_attenuation.y[i]);
        __m256 cur_vp_attenuation_z = _mm256_load_ps(&pixel_datas->cur_vp_attenuation.z[i]);
        __m256 radius = _mm256_load_ps(&pixel_datas->radius.data[i]);
        __m256 cur_flux_x = _mm256_load_ps(&pixel_datas->cur_flux.x[i]);
        __m256 cur_flux_y = _mm256_load_ps(&pixel_datas->cur_flux.y[i]);
        __m256 cur_flux_z = _mm256_load_ps(&pixel_datas->cur_flux.z[i]);
        __m256 tau_x = _mm256_load_ps(&pixel_datas->tau.x[i]);
        __m256 tau_y = _mm256_load_ps(&pixel_datas->tau.y[i]);
        __m256 tau_z = _mm256_load_ps(&pixel_datas->tau.z[i]);

        __m256 cur_photons = _mm256_load_ps(&pixel_datas->cur_photons.data[i]);
        __m256 num_photons = _mm256_load_ps(&pixel_datas->num_photons.data[i]);

        __m256 mask = _mm256_cmp_ps(cur_photons,  _mm256_setzero_ps(), _CMP_GT_OQ);

        __m256 new_num_photons = _mm256_fmadd_ps(alpha_vector, cur_photons, num_photons);
        __m256 new_radius = _mm256_mul_ps(radius, _mm256_sqrt_ps(_mm256_div_ps(new_num_photons, _mm256_add_ps(num_photons, cur_photons))));
        __m256 multiplier = _mm256_div_ps(_mm256_mul_ps(new_radius, new_radius), _mm256_mul_ps(radius, radius));
        __m256 t0_x = _mm256_mul_ps(multiplier, _mm256_fmadd_ps(cur_vp_attenuation_x, cur_flux_x, tau_x));
        __m256 t0_y = _mm256_mul_ps(multiplier, _mm256_fmadd_ps(cur_vp_attenuation_y, cur_flux_y, tau_y));
        __m256 t0_z = _mm256_mul_ps(multiplier, _mm256_fmadd_ps(cur_vp_attenuation_z, cur_flux_z, tau_z));

        _mm256_store_ps(&pixel_datas->tau.x[i], _mm256_blendv_ps(tau_x, t0_x, mask));
        _mm256_store_ps(&pixel_datas->tau.y[i], _mm256_blendv_ps(tau_y, t0_y, mask));
        _mm256_store_ps(&pixel_datas->tau.z[i], _mm256_blendv_ps(tau_z, t0_z, mask));
        _mm256_store_ps(&pixel_datas->num_photons.data[i], _mm256_blendv_ps(num_photons, new_num_photons, mask));
        _mm256_store_ps(&pixel_datas->radius.data[i], _mm256_blendv_ps(radius, new_radius, mask));

        _mm256_store_ps(&pixel_datas->cur_photons.data[i], _mm256_setzero_ps());
        _mm256_store_ps(&pixel_datas->cur_flux.x[i], _mm256_setzero_ps());
        _mm256_store_ps(&pixel_datas->cur_flux.y[i], _mm256_setzero_ps());
        _mm256_store_ps(&pixel_datas->cur_flux.z[i], _mm256_setzero_ps());
    }

    for (; i < pixel_datas->size; i++) {
        float cur_vp_attenuation_x = pixel_datas->cur_vp_attenuation.x[i];
        float cur_vp_attenuation_y = pixel_datas->cur_vp_attenuation.y[i];
        float cur_vp_attenuation_z = pixel_datas->cur_vp_attenuation.z[i];
        float radius = pixel_datas->radius.data[i];
        float cur_flux_x = pixel_datas->cur_flux.x[i];
        float cur_flux_y = pixel_datas->cur_flux.y[i];
        float cur_flux_z = pixel_datas->cur_flux.z[i];
        float tau_x = pixel_datas->tau.x[i];
        float tau_y = pixel_datas->tau.y[i];
        float tau_z = pixel_datas->tau.z[i];
        float cur_photons = pixel_datas->cur_photons.data[i];
        float num_photons = pixel_datas->num_photons.data[i];
        if (cur_photons > 0) {
            float new_num_photons = num_photons + 1.0f * alpha * cur_photons;
            float new_radius = radius * sqrtf(new_num_photons / (num_photons + cur_photons));
            float multiplier = (new_radius * new_radius) / (radius * radius);
            float t0_x = multiplier * (tau_x + (cur_vp_attenuation_x * cur_flux_x));
            float t0_y = multiplier * (tau_y + (cur_vp_attenuation_y * cur_flux_y));
            float t0_z = multiplier * (tau_z + (cur_vp_attenuation_z * cur_flux_z));
            pixel_datas->tau.x[i] = t0_x;
            pixel_datas->tau.y[i] = t0_y;
            pixel_datas->tau.z[i] = t0_z;
            pixel_datas->num_photons.data[i] = new_num_photons;
            pixel_datas->radius.data[i] = new_radius;
        }
        pixel_datas->cur_photons.data[i] = 0;
        pixel_datas->cur_flux.x[i] = 0;
        pixel_datas->cur_flux.y[i] = 0;
        pixel_datas->cur_flux.z[i] = 0;
    }
}

void sppm_store(PixelData *pixel_datas, int num_iters, int num_photons, int H, int W, Bitmap *bitmap) {
    int num_photons_total = num_iters * num_photons;
    printf("num_photons_total: %d\n", num_photons_total);

    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int idx = i * W + j;
            float radius = pixel_datas->radius.data[idx];
            float tau_x = pixel_datas->tau.x[idx];
            float tau_y = pixel_datas->tau.y[idx];
            float tau_z = pixel_datas->tau.z[idx];
            float direct_radiance_x = pixel_datas->direct_radiance.x[idx] / num_iters;
            float direct_radiance_y = pixel_datas->direct_radiance.y[idx] / num_iters;
            float direct_radiance_z = pixel_datas->direct_radiance.z[idx] / num_iters;

            float inv_mult = 1 / (M_PI * num_photons_total * radius * radius);
            Vector total_radiance = {
                    .x = direct_radiance_x + tau_x * inv_mult,
                    .y = direct_radiance_y + tau_y * inv_mult,
                    .z = direct_radiance_z + tau_z * inv_mult
            };
            bitmap_set(bitmap, j, i, &total_radiance);
        }
    }
}

void sppm_render(SPPM *sppm, Bitmap *bitmap) {
    fprintf(stderr, "_SPPM_RADIUS_MULT = %f; \n", _SPPM_RADIUS_MULT);
//    initialise data
    int W, H;
    assert(sppm->camera->W == bitmap->W && sppm->camera->H == bitmap->H);
    W = sppm->camera->W;
    H = sppm->camera->H;

    // Init launch indices
    {
        size_t size = W * H * sizeof(float), ind = 0;
        sppm->launch_indices_x = (float*) aligned_alloc(32, size);
        sppm->launch_indices_y = (float*) aligned_alloc(32, size);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                sppm->launch_indices_x[ind] = (float) x;
                sppm->launch_indices_y[ind] = (float) y;
                ind++;
            }
        }
    }

    int num_iterations = sppm->num_iterations;
    PixelData pixel_datas;
    sppm_pixel_data_init(&pixel_datas, H * W);

    int i;
    __m256 initial_radius_vector = _mm256_set1_ps(sppm->initial_radius);
    for (i = 0; i < pixel_datas.size_float_simd; i += NUM_FLOAT_SIMD) {
        _mm256_store_ps(&pixel_datas.radius.data[i], initial_radius_vector);
    }
    for (; i < pixel_datas.size; i++) {
        pixel_datas.radius.data[i] = sppm->initial_radius;
    }

//    Loop
    struct PixelDataLookup lookup;
    sppm_pixel_data_lookup_init(&lookup, H * W);

    for (int i = 0; i < num_iterations; i++) {
        clock_t start;
        float elapse;
#define tic start = clock()
#define toc elapse = (float) (clock() - start) / CLOCKS_PER_SEC; fprintf(stderr, "\t%f sec\n", elapse)

        fprintf(stderr, "Current %d out of %d\n", i, num_iterations);
        fprintf(stderr, "\tCamera pass ");
        tic, sppm_camera_pass(sppm, &pixel_datas), toc;

        fprintf(stderr, "\tBuild lookup");
        tic, sppm_build_pixel_data_lookup(&lookup, &pixel_datas), toc;

        fprintf(stderr, "\tPhoton pass ");
        tic, sppm_photon_pass(sppm, &lookup, &pixel_datas), toc;

        fprintf(stderr, "\tConsolidate ");
        tic, sppm_consolidate(&pixel_datas, sppm->alpha), toc;

#undef tic
#undef toc
    }
    sppm_store(&pixel_datas, num_iterations, sppm->num_photons, H, W, bitmap);

    free(sppm->launch_indices_x);
    free(sppm->launch_indices_y);

    sppm_pixel_data_lookup_free(&lookup);
    sppm_pixel_data_free(&pixel_datas);
}
