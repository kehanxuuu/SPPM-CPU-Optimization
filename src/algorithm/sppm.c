#include "sppm.h"
#include <time.h>

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

void sppm_pixel_data_init(PixelData *pixel_datas, size_t size) {
    arr_init_float(&pixel_datas->radius, size, size);
    arr_init_int(&pixel_datas->num_photons, size, size);
    arr_init_vector(&pixel_datas->tau, size, size);
    arr_init_vector(&pixel_datas->direct_radiance, size, size);
    arr_init_int(&pixel_datas->cur_photons, size, size);
    arr_init_vector(&pixel_datas->cur_flux, size, size);
    arr_init_vector(&pixel_datas->cur_vp_attenuation, size, size);
    arr_init(&pixel_datas->cur_vp_intersection, size, size, sizeof(Intersection));

    // initialize with zero
    memset(pixel_datas->num_photons.data, 0, size * sizeof(int));
    memset(pixel_datas->tau.data, 0, size * sizeof(Vector));
    memset(pixel_datas->direct_radiance.data, 0, size * sizeof(Vector));
    memset(pixel_datas->cur_photons.data, 0, size * sizeof(int));
    memset(pixel_datas->cur_flux.data, 0, size * sizeof(Vector));
}

void sppm_pixel_data_lookup_init(PixelDataLookup *lookup, size_t init_size) {
    lookup->fixed_size = init_size;
    lookup->hash_table = malloc(sizeof(IntArray) * init_size);
    for(int i = 0; i < init_size; i++) {
        arr_init_int(&lookup->hash_table[i], 20, 0);
    }
}

void sppm_pixel_data_lookup_assign(PixelDataLookup *lookup, float grid_size, Vector grid_min, Vector grid_max){
    lookup->grid_res = grid_size;
    lookup->grid_min = grid_min;
    lookup->grid_max = grid_max;
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
    arr_free(&pixel_datas->cur_vp_intersection);
}

size_t sppm_pixel_data_lookup_hash(PixelDataLookup *lookup, Vector3u *loc) {
    size_t hash_val = ((loc->x * 18397) + (loc->y * 20483) + (loc->z * 29303)) % lookup->fixed_size;
    return hash_val;
}

Vector3u sppm_pixel_data_lookup_to_grid(PixelDataLookup *lookup, Vector *loc) {
    Vector from_min = vv_sub(loc, &lookup->grid_min);
    size_t loc_x = (size_t) fmaxf(0, from_min.x / lookup->grid_res);
    size_t loc_y = (size_t) fmaxf(0, from_min.y / lookup->grid_res);
    size_t loc_z = (size_t) fmaxf(0, from_min.z / lookup->grid_res);
    return (Vector3u) {loc_x, loc_y, loc_z};
}

void sppm_pixel_data_lookup_store(PixelDataLookup *lookup, Vector3u *loc_3d, int pd_index) {
    size_t ht_loc = sppm_pixel_data_lookup_hash(lookup, loc_3d);
    arr_add_int(&lookup->hash_table[ht_loc], &pd_index);
}

void sppm_build_pixel_data_lookup(PixelDataLookup *lookup, PixelData *pixel_datas, float* radius_cache, Vector* position_cache, size_t H, size_t W) {
//    grid data computation
    Vector grid_min = (Vector) {FLT_MAX, FLT_MAX, FLT_MAX};
    Vector grid_max = (Vector) {-FLT_MAX, -FLT_MAX, -FLT_MAX};
#if _SPPM_RADIUS_TYPE == 0
    float max_radius = -FLT_MAX;
#elif _SPPM_RADIUS_TYPE == 1
    float avg_radius = 0;
#endif

    FloatArray radii = pixel_datas->radius;
    VectorArray attenuation_array = pixel_datas->cur_vp_attenuation;
    Array cur_vp_intersection = pixel_datas->cur_vp_intersection;

    int branch_cache[W];
    int k = 0;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int idx = i * W + j;
            float radius_f = arr_get_float(&radii, idx);
            Vector attenuation = arr_get_vector(&attenuation_array, idx);

            if (vv_equal(&attenuation, &ZERO_VEC)) {
                branch_cache[k] = idx;
                k++;
                continue;
            }


            Intersection *isect = arr_get(&cur_vp_intersection, idx);
            Vector pos = isect->p;

            Vector radius = (Vector) {radius_f, radius_f, radius_f};
            Vector pos_min = vv_sub(&pos, &radius);
            Vector pos_max = vv_add(&pos, &radius);
            grid_min = vv_min(&grid_min, &pos_min);
            grid_max = vv_max(&grid_max, &pos_max);
#if _SPPM_RADIUS_TYPE == 0
            max_radius = fmaxf(max_radius, radius_f);
#elif _SPPM_RADIUS_TYPE == 1
            avg_radius += pd->radius;
#endif
            radius_cache[idx] = radius_f;
            position_cache[idx] = pos;
        }
    }
    sppm_pixel_data_lookup_clear(lookup);
#if _SPPM_RADIUS_TYPE == 0
    sppm_pixel_data_lookup_assign(lookup, _SPPM_RADIUS_MULT * max_radius, grid_min, grid_max);
#elif _SPPM_RADIUS_TYPE == 1
    avg_radius /= (float) (pixel_datas->height * pixel_datas->width);
	sppm_pixel_data_lookup_assign(lookup, _SPPM_RADIUS_MULT * avg_radius, grid_min, grid_max);
#endif


//    build grid
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int idx = i * W + j;
            if (idx == branch_cache[k]) {
                k++;
                continue;
            }

            float radius_f = radius_cache[idx];
            Vector pos = position_cache[idx];

            Vector radius = (Vector) {radius_f, radius_f, radius_f};
            Vector pos_min = vv_sub(&pos, &radius);
            Vector pos_max = vv_add(&pos, &radius);
            Vector3u from_loc_3d = sppm_pixel_data_lookup_to_grid(lookup, &pos_min);
            Vector3u to_loc_3d = sppm_pixel_data_lookup_to_grid(lookup, &pos_max);
            for (size_t x = from_loc_3d.x; x <= to_loc_3d.x; x++) {
                for (size_t y = from_loc_3d.y; y <= to_loc_3d.y; y++) {
                    for (size_t z = from_loc_3d.z; z <= to_loc_3d.z; z++) {
                        Vector3u cur_loc = (Vector3u) {x, y, z};
                        sppm_pixel_data_lookup_store(lookup, &cur_loc, idx);
                    }
                }
            }
        }
    }
}

void sppm_camera_pass_pixel(SPPM *sppm, int x, int y, Vector* direct_radiance, Vector* vp_attenuation, Intersection* vp_intersection) {
    Ray ray = generate_ray(sppm->camera, x, y, (Vector2f) {randf(), randf()});

    Vector attenuation = {1.0f, 1.0f, 1.0f};
    *vp_attenuation = ZERO_VEC;

    *direct_radiance = ZERO_VEC;

    for (int i = 0; i < sppm->ray_max_depth; i++) {
        Intersection isect;
        if (!scene_intersect(sppm->scene, &ray, &isect)) {
            vvv_fmaeq(direct_radiance, &attenuation, &sppm->background);
            return;
        }

        // Emission
        vvv_fmaeq(direct_radiance, &attenuation, &isect.hit->emission);

        if (isect.hit->material == DIFFUSE) {
            // Compute direct lighting and break loop when hitting any diffuse surface
            Vector Ld = estimate_direct_lighting(sppm->scene, &isect);
            vvv_fmaeq(direct_radiance, &attenuation, &Ld);
            *vp_intersection = isect;
            *vp_attenuation = attenuation;
            return;
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
            return;
        }

        vv_muleq(&attenuation, &cur_attenuation);
        float continue_prob = v_cwise_max(&attenuation);
        // Russian Roulette
        if (continue_prob < 0.25) {
            if (randf() > continue_prob)
                return;
            vs_diveq(&attenuation, continue_prob);
        }

        ray = (Ray) {isect.p, isect.wo, INFINITY};
        ray.o = ray_at(&ray, EPSILON);
    }

    return;
}

void sppm_camera_pass(SPPM *sppm, PixelData *pixel_datas) {
    size_t W, H;
    W = sppm->camera->W;
    H = sppm->camera->H;
    pixel_datas->cur_vp_intersection.size = 0;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int idx = i * W + j;
            Vector direct_luminance, vp_attenuation;
            Intersection vp_intersection;
            sppm_camera_pass_pixel(sppm, j, i, &direct_luminance, &vp_attenuation, &vp_intersection);
            arr_set_add_vector(&pixel_datas->direct_radiance, idx, direct_luminance);
            arr_set_vector(&pixel_datas->cur_vp_attenuation, idx, vp_attenuation);
            arr_add(&pixel_datas->cur_vp_intersection, &vp_intersection);
        }
    }
}

void sppm_photon_pass_photon(SPPM *sppm, PixelDataLookup *lookup, PixelData *pixel_datas) {
    Ray ray;
    float pdf_emitter, pdf_pos, pdf_dir;
    Mesh *emitter = sample_emitter(sppm->scene, randf(), &pdf_emitter);
    switch (emitter->geometry->type) {
        case SPHERE:
            ray = sphere_surface_photon_sample((Sphere *) emitter->geometry->data, (Vector2f) {randf(), randf()}, (Vector2f) {randf(), randf()},
                                               &pdf_pos, &pdf_dir);
            break;
        default:
        UNIMPLEMENTED;
    }

    Vector light_radiance = vs_div(&emitter->emission, pdf_emitter * pdf_pos * pdf_dir);

    Array cur_vp_intersection_array = pixel_datas->cur_vp_intersection;
    FloatArray radii = pixel_datas->radius;
    VectorArray cur_flux_array = pixel_datas->cur_flux;
    IntArray cur_photons_array = pixel_datas->cur_photons;
    for (int i = 0; i < sppm->ray_max_depth; i++) {
        Intersection isect;
        if (!scene_intersect(sppm->scene, &ray, &isect)) {
            break;
        }
        Vector wo = vs_mul(&ray.d, -1);

        if (i > 0) {  // Direct illumination is accounted for in the camera pass
            Vector3u loc_3d = sppm_pixel_data_lookup_to_grid(lookup, &isect.p);
            size_t ht_loc = sppm_pixel_data_lookup_hash(lookup, &loc_3d);
            for(int cur_arr_ind = 0; cur_arr_ind < lookup->hash_table[ht_loc].size; cur_arr_ind++) {
                int pd_index = arr_get_int(&lookup->hash_table[ht_loc], cur_arr_ind);
                Intersection *cur_vp_intersection = arr_get(&cur_vp_intersection_array, pd_index);
                float radius = arr_get_float(&radii, pd_index);
                Vector dist_between = vv_sub(&cur_vp_intersection->p, &isect.p);
                if (v_norm_sqr(&dist_between) < radius * radius) {
                    cur_vp_intersection->wo = wo;
                    // Only contribute energy to diffuse materials
                    if (cur_vp_intersection->hit->material == DIFFUSE) {
                        // Vector bsdf = bsdf_eval_diffuse(cur_vp_intersection);
                        // manual inline bsdf_eval_diffuse
                        Vector bsdf = ZERO_VEC;
                        if (vv_dot(&cur_vp_intersection->wi, &cur_vp_intersection->n) < 0 && vv_dot(&cur_vp_intersection->wo, &cur_vp_intersection->n) > 0)
                            bsdf = vs_mul(&cur_vp_intersection->hit->albedo, INV_PI);

                        arr_set_add_vector(&cur_flux_array, pd_index, vv_mul(&bsdf, &light_radiance)); // flux += bsdf * L
                        arr_set_add_int(&cur_photons_array, pd_index, 1);
                    }
                    else {
                        UNREACHABLE;
                    }
                }
            }
        }

        Vector cur_attenuation = bsdf_sample(&isect, (Vector2f) {randf(), randf()});
        vv_muleq(&light_radiance, &cur_attenuation);
        float continue_prob = v_cwise_max(&light_radiance);
        // Russian Roulette
        if (continue_prob < 0.25) {
            if (randf() > continue_prob) {
                break;
            }
            vs_diveq(&light_radiance, continue_prob);
        }

        ray = (Ray) {isect.p, isect.wo, INFINITY};
    }
}

void sppm_photon_pass(SPPM *sppm, PixelDataLookup *lookup, PixelData *pixel_datas) {
    for (int i = 0; i < sppm->num_photons; i++) {
        sppm_photon_pass_photon(sppm, lookup, pixel_datas);
    }
}

void sppm_consolidate(PixelData *pixel_datas, float alpha, size_t H, size_t W) {
    VectorArray cur_vp_attenuation_array = pixel_datas->cur_vp_attenuation;
    FloatArray radii = pixel_datas->radius;
    VectorArray cur_flux_array = pixel_datas->cur_flux;
    VectorArray tau_array = pixel_datas->tau;
    IntArray cur_photons_array = pixel_datas->cur_photons;
    IntArray num_photons_array = pixel_datas->num_photons;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int idx = i * W + j;
            Vector cur_vp_attenuation = arr_get_vector(&cur_vp_attenuation_array, idx);
            float radius = arr_get_float(&radii, idx);
            Vector cur_flux = arr_get_vector(&cur_flux_array, idx);
            Vector tau = arr_get_vector(&tau_array, idx);
            int cur_photons = arr_get_int(&cur_photons_array, idx);
            int num_photons = arr_get_int(&num_photons_array, idx);
            if (cur_photons > 0) {
                float new_num_photons = num_photons + 1.0f * alpha * cur_photons;
                float new_radius = radius * sqrtf(new_num_photons / (num_photons + cur_photons));
                {
                    float multiplier = (new_radius * new_radius) / (radius * radius);
                    Vector tv1 = vv_mul(&cur_vp_attenuation, &cur_flux);
                    Vector tv2 = vv_add(&tau, &tv1);
                    arr_set_vector(&tau_array, idx, vs_mul(&tv2, multiplier));
                }
                arr_set_int(&num_photons_array, idx, new_num_photons);
                arr_set_float(&radii, idx, new_radius);
            }
        }
    }
    memset(pixel_datas->cur_photons.data, 0, H * W * sizeof(int));
    memset(pixel_datas->cur_flux.data, 0, H * W * sizeof(Vector));
}

void sppm_store(PixelData *pixel_datas, int num_iters, int num_photons, size_t H, size_t W, Bitmap *bitmap) {
    int num_photons_total = num_iters * num_photons;
    printf("num_photons_total: %d\n", num_photons_total);

    VectorArray direct_radiance_array = pixel_datas->direct_radiance;
    VectorArray tau_array = pixel_datas->tau;
    FloatArray radii = pixel_datas->radius;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int idx = i * W + j;
            float radius = arr_get_float(&radii, idx);
            Vector tau = arr_get_vector(&tau_array, idx);
            Vector direct_radiance = arr_get_vector(&direct_radiance_array, idx);
            vs_diveq(&direct_radiance, num_iters);
            Vector indirect_radiance = vs_div(&tau, M_PI * num_photons_total * radius * radius);
            Vector total_radiance = vv_add(&direct_radiance, &indirect_radiance);
            bitmap_set(bitmap, j, i, &total_radiance);
        }
    }

}

void sppm_render(SPPM *sppm, Bitmap *bitmap) {
    fprintf(stderr, "_SPPM_RADIUS_MULT = %f; _SPPM_RADIUS_TYPE = %d\n", _SPPM_RADIUS_MULT, _SPPM_RADIUS_TYPE);
//    initialise data
    size_t W, H;
    W = sppm->camera->W;
    H = sppm->camera->H;
    int num_iterations = sppm->num_iterations;
    PixelData pixel_datas;
    sppm_pixel_data_init(&pixel_datas, H * W);
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            arr_set_float(&pixel_datas.radius, i * W + j, sppm->initial_radius);
        }
    }
//    Loop
    float* radius_cache = malloc(H*W*sizeof(float));
    Vector* position_cache = malloc(H*W*sizeof(Vector));
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
        tic, sppm_build_pixel_data_lookup(&lookup, &pixel_datas, radius_cache, position_cache, H, W), toc;

        fprintf(stderr, "\tPhoton pass ");
        tic, sppm_photon_pass(sppm, &lookup, &pixel_datas), toc;

        fprintf(stderr, "\tConsolidate ");
        tic, sppm_consolidate(&pixel_datas, sppm->alpha, H, W), toc;

#undef tic
#undef toc
    }
    sppm_store(&pixel_datas, num_iterations, sppm->num_photons, H, W, bitmap);

    sppm_pixel_data_lookup_free(&lookup);
    sppm_pixel_data_free(&pixel_datas);

    free(radius_cache);
    free(position_cache);
}
