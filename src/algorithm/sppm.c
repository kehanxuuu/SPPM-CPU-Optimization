#include "sppm.h"

void sppm_init(SPPM *sppm, int num_iterations, int ray_max_depth, int photon_num_iter, float initial_radius, Scene* scene, Camera* camera, Vector* background){
    sppm->num_iterations = num_iterations;
    sppm->ray_max_depth = ray_max_depth;
    sppm->num_photons = photon_num_iter;
    sppm->initial_radius = initial_radius;
    sppm->alpha = 2.0f / 3.0f;
    sppm->background = *background;
    sppm->scene = scene;
    sppm->camera = camera;
}

void sppm_pixel_data_lookup_init(PixelDataLookup* lookup, size_t init_size, float grid_size, Vector grid_min, Vector grid_max){
    lookup->fixed_size = init_size;
//    NULL the next pointers
    lookup->hash_table = calloc(sizeof(PixelDataLookupNode), init_size);
    lookup->grid_res = grid_size;
    lookup->grid_min = grid_min;
    lookup->grid_max = grid_max;
}

void sppm_pixel_data_lookup_free(PixelDataLookup* lookup){
    for(int i = 0; i < lookup->fixed_size; i++){
        PixelDataLookupNode* cur_node = lookup->hash_table[i].next;
        while(cur_node != NULL){
            PixelDataLookupNode* prev_node = cur_node;
            cur_node = prev_node->next;
            free(prev_node);
        }
    }
    free(lookup->hash_table);
}

size_t sppm_pixel_data_lookup_hash(PixelDataLookup* lookup, Vector3u* loc){
    size_t hash_val = ((loc->x * 18397) + (loc->y * 20483) + (loc->z * 29303)) % lookup->fixed_size;
    return hash_val;
}

Vector3u sppm_pixel_data_lookup_to_grid(PixelDataLookup* lookup, Vector* loc) {
    Vector from_min = vv_sub(loc, &lookup->grid_min);
    size_t loc_x = (size_t) (from_min.x / lookup->grid_res);
    size_t loc_y = (size_t) (from_min.y / lookup->grid_res);
    size_t loc_z = (size_t) (from_min.z / lookup->grid_res);
    return (Vector3u){loc_x, loc_y, loc_z};
}

void sppm_pixel_data_lookup_store(PixelDataLookup* lookup, Vector3u* loc_3d, PixelData* pd){
    size_t ht_loc = sppm_pixel_data_lookup_hash(lookup, loc_3d);
    PixelDataLookupNode* prev_node = &lookup->hash_table[ht_loc];
    while(prev_node->next != NULL){
        prev_node = prev_node->next;
    }
    PixelDataLookupNode* cur_node = malloc(sizeof(PixelDataLookupNode));
    prev_node->next = cur_node;
    cur_node->next = NULL;
    cur_node->content = pd;
}

void sppm_build_pixel_data_lookup(PixelDataLookup* lookup, ArrayFixed2D* pixel_datas){
//    grid data computation
    Vector grid_min = (Vector){FLT_MAX, FLT_MAX, FLT_MAX};
    Vector grid_max = (Vector){-FLT_MAX, -FLT_MAX, -FLT_MAX};
    float max_radius = -FLT_MAX;
    for(int i = 0; i < pixel_datas->height; i++) {
        for (int j = 0; j < pixel_datas->width; j++) {
            PixelData *pd = (PixelData *) arrfixed2d_get(pixel_datas, i, j);
            if(vv_equal(&pd->cur_vp.attenuation, &ZERO_VEC)){
                continue;
            }
            Vector *pos = &pd->cur_vp.intersection.p;

            Vector radius = (Vector){pd->radius, pd->radius, pd->radius};
            Vector pos_min = vv_sub(pos, &radius);
            Vector pos_max = vv_add(pos, &radius);
            grid_min = vv_min(&grid_min, &pos_min);
            grid_max = vv_max(&grid_max, &pos_max);
            max_radius = fmaxf(max_radius, pd->radius);
        }
    }
    sppm_pixel_data_lookup_init(lookup, pixel_datas->arr.size, max_radius, grid_min, grid_max);

//    build grid
    for(int i = 0; i < pixel_datas->height; i++){
        for(int j = 0; j < pixel_datas->width; j++){
            PixelData* pd = (PixelData*)arrfixed2d_get(pixel_datas, i, j);
            if(vv_equal(&pd->cur_vp.attenuation, &ZERO_VEC)){
                continue;
            }
            Vector *pos = &pd->cur_vp.intersection.p;

            Vector radius = (Vector){pd->radius, pd->radius, pd->radius};
            Vector pos_min = vv_sub(pos, &radius);
            Vector pos_max = vv_add(pos, &radius);
            Vector3u from_loc_3d = sppm_pixel_data_lookup_to_grid(lookup, &pos_min);
            Vector3u to_loc_3d = sppm_pixel_data_lookup_to_grid(lookup, &pos_max);
            for(size_t x = from_loc_3d.x; x <= to_loc_3d.x; x++){
                for(size_t y = from_loc_3d.y; y <= to_loc_3d.y; y++){
                    for(size_t z = from_loc_3d.z; z <= to_loc_3d.z; z++){
                        Vector3u cur_loc = (Vector3u){x, y, z};
                        sppm_pixel_data_lookup_store(lookup, &cur_loc, pd);
                    }
                }
            }
        }
    }
}

void sppm_camera_pass_pixel(SPPM *sppm, int x, int y, PixelData* pd) {
    Ray ray = generate_ray(sppm->camera, x, y, (Vector2f){randf(), randf()});

    Vector attenuation = {1.0f, 1.0f, 1.0f};
    pd->cur_vp.attenuation = ZERO_VEC;

    bool was_specular = true;
    for(int i = 0; i < sppm->ray_max_depth; i++){
        Intersection isect;
        if (!scene_intersect(sppm->scene, &ray, &isect)) {
            Vector scaled_background = vv_mul(&sppm->background, &attenuation);
            vv_addeq(&pd->direct_radiance, &scaled_background);
            break;
        }

        if(was_specular){
            Vector emitted_radiance = vv_mul(&isect.hit->emission, &attenuation);
            vv_addeq(&pd->direct_radiance, &emitted_radiance);
        }

//      TODO: direct illumination

        Vector cur_attenuation;
        if(isect.hit->material == DIFFUSE){
            pd->cur_vp = (VisiblePoint){isect, attenuation};
            break;
        }else if(isect.hit->material == SPECULAR){
            cur_attenuation = bsdf_sample_specular(&isect, (Vector2f){randf(), randf()});
        }else{
            assert(false);
        }
        if(vv_equal(&cur_attenuation, &ZERO_VEC)){
            break;
        }

        was_specular = isect.hit->material == SPECULAR;
        vv_muleq(&attenuation, &cur_attenuation);
        float max_attenuation = v_cwise_max(&attenuation);
        // Russian Roulette
        if (max_attenuation < 0.25) {
            float continue_prob = fminf(1.0f, max_attenuation);
            if (randf() > continue_prob) {
                break;
            }
            vs_diveq(&attenuation, continue_prob);
        }

        ray = (Ray){isect.p, isect.wo, INFINITY};
    }
}

void sppm_camera_pass(SPPM *sppm, ArrayFixed2D* pixel_datas){
    size_t W, H;
    W = sppm->camera->W;
    H = sppm->camera->H;
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            PixelData* addr = (PixelData *)arrfixed2d_get(pixel_datas, i, j);
            sppm_camera_pass_pixel(sppm, i, j, addr);
        }
    }
}

void sppm_photon_pass_photon(SPPM *sppm, PixelDataLookup* lookup) {
    float sample_float = randf();
    size_t selected_mesh;
    for(int i = 0; i < sppm->emitters.prefix_intensity.size - 1; i++){
        if(sample_float >= *(float*)arr_get(&sppm->emitters.prefix_intensity, i) &&
           sample_float < *(float*)arr_get(&sppm->emitters.prefix_intensity, i + 1)){
            selected_mesh = *(size_t *)arr_get(&sppm->emitters.emitters, i);
            break;
        }
    }
    Ray ray;
    Mesh *mesh = scene_get(sppm->scene, selected_mesh);
    if(mesh->geometry->type == SPHERE){
        ray = sphere_surface_sample((Sphere*)mesh->geometry->data, (Vector2f){randf(), randf()}, (Vector2f){randf(), randf()});
    }else {
        assert(false);
    }

    Vector attenuation = mesh->emission;

    for(int i = 0; i < sppm->ray_max_depth; i++){
        Intersection isect;
        if (!scene_intersect(sppm->scene, &ray, &isect)) {
            break;
        }

        // Direct illumination is accounted for in the camera pass
        if (i > 0) {
            Vector3u loc_3d = sppm_pixel_data_lookup_to_grid(lookup, &isect.p);
            size_t ht_loc = sppm_pixel_data_lookup_hash(lookup, &loc_3d);
            PixelDataLookupNode* cur_node = lookup->hash_table[ht_loc].next;
            while(cur_node != NULL){
                PixelData* pd = cur_node->content;
                Vector dist_between = vv_sub(&pd->cur_vp.intersection.p, &isect.p);
                if(v_norm_sqr(&dist_between) < pd->radius * pd->radius){
                    pd->cur_vp.intersection.wo = vv_sub(&ZERO_VEC, &ray.d);
                    if(pd->cur_vp.intersection.hit->material == DIFFUSE){
                        Vector bsdf = bsdf_eval_diffuse(&pd->cur_vp.intersection);
                        vvv_fmaeq(&pd->cur_flux, &bsdf, &attenuation);  // flux += bsdf * atten
                        pd->cur_photons++;
                    }else if(pd->cur_vp.intersection.hit->material == SPECULAR){
                        assert(false);
                    }else {
                        assert(false);
                    }
                }
                cur_node = cur_node->next;
            }
        }
        Vector cur_attenuation;
        if(isect.hit->material == DIFFUSE){
            cur_attenuation = bsdf_sample_diffuse(&isect, (Vector2f){randf(), randf()});
        }else if(isect.hit->material == SPECULAR){
            cur_attenuation = bsdf_sample_specular(&isect, (Vector2f){randf(), randf()});
        }else{
            assert(false);
        }

        vv_muleq(&attenuation, &cur_attenuation);
        float max_attenuation = v_cwise_max(&attenuation);
        // Russian Roulette
        if (max_attenuation < 0.25) {
            float continue_prob = fminf(1.0f, max_attenuation);
            if (randf() > continue_prob) {
                break;
            }
            vs_diveq(&attenuation, continue_prob);
        }

        ray = (Ray){isect.p, isect.wo, INFINITY};
    }
}

void sppm_photon_pass(SPPM *sppm, PixelDataLookup* lookup){
    for(int i = 0; i < sppm->num_photons; i++){
        sppm_photon_pass_photon(sppm, lookup);
    }
}

void sppm_consolidate(SPPM *sppm, ArrayFixed2D* pixel_datas){
    for(int i = 0; i < pixel_datas->height; i++){
        for(int j = 0; j < pixel_datas->width; j++) {
            PixelData* pd = (PixelData*) arrfixed2d_get(pixel_datas, i, j);
            if(pd->cur_photons > 0) {
                float new_num_photons = pd->num_photons + 1.0f * sppm->alpha * pd->cur_photons;
                float new_radius = pd->radius * sqrtf(new_num_photons / (pd->num_photons + pd->cur_photons));
                {
                    float multiplier = (new_radius * new_radius) / (pd->radius * pd->radius);
                    Vector tv1 = vv_mul(&pd->cur_vp.attenuation, &pd->cur_flux);
                    Vector tv2 = vv_add(&pd->tau, &tv1);
                    pd->tau = vs_mul(&tv2, multiplier);
                }
                pd->num_photons = new_num_photons;
                pd->radius = new_radius;
            }

        }
    }
}

void sppm_store(SPPM *sppm, ArrayFixed2D* pixel_datas, int num_iters, Bitmap* bitmap){
    size_t W, H;
    W = sppm->camera->W;
    H = sppm->camera->H;
    int num_photons_total = num_iters * sppm->num_photons;
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            PixelData* pd = (PixelData*) arrfixed2d_get(pixel_datas, i, j);
            Vector direct_radiance = vs_div(&pd->direct_radiance, num_iters);
            Vector indirect_radiance = vs_div(&pd->tau, M_PI * num_photons_total * pd->radius * pd->radius);
            Vector total_radiance = vv_add(&direct_radiance, &indirect_radiance);
            bitmap_set(bitmap, i, j, &total_radiance);
        }
    }

}

void sppm_render(SPPM* sppm, Bitmap* bitmap){
//    initialise data
    size_t W, H;
    W = sppm->camera->W;
    H = sppm->camera->H;
    ArrayFixed2D pixel_datas;
    arrfixed2d_init(&pixel_datas, H, W, sizeof(PixelData));
    for(int i = 0; i < pixel_datas.height; i++){
        for(int j = 0; j < pixel_datas.width; j++) {
            PixelData* pd = (PixelData*) arrfixed2d_get(&pixel_datas, i, j);
            pd->radius = sppm->initial_radius;
        }
    }

//    Obtain emitters;
    arr_init(&sppm->emitters.emitters, 20, 0, sizeof(size_t));
    arr_init(&sppm->emitters.prefix_intensity, 20, 0, sizeof(float));
    float cur_sum = 0;
    arr_add(&sppm->emitters.prefix_intensity, &cur_sum);
    for(size_t i = 0; i < sppm->scene->meshes.size; i++) {
        Mesh *cur_mesh = scene_get(sppm->scene, i);
        if (vv_equal(&cur_mesh->emission, &ZERO_VEC)) {
            continue;
        }
        cur_sum += v_norm(&cur_mesh->emission);
        arr_add(&sppm->emitters.emitters, &i);
        arr_add(&sppm->emitters.prefix_intensity, &cur_sum);
    }
    for(int i = 0; i < sppm->emitters.prefix_intensity.size; i++) {
        *(float*)arr_get(&sppm->emitters.prefix_intensity, i) /= cur_sum;
    }

//    Loop
    for(int i = 0; i < sppm->num_iterations; i++){
        fprintf(stderr, "Current %d out of %d\n", i, sppm->num_iterations);
        sppm_camera_pass(sppm, &pixel_datas);
        fprintf(stderr, "\tLookup\n");
        struct PixelDataLookup lookup;
        sppm_build_pixel_data_lookup(&lookup, &pixel_datas);
        fprintf(stderr, "\tPhoton\n");
        sppm_photon_pass(sppm, &lookup);
        fprintf(stderr, "\tconsolidate\n");
        sppm_consolidate(sppm, &pixel_datas);
        sppm_pixel_data_lookup_free(&lookup);
    }
    sppm_store(sppm, &pixel_datas, sppm->num_iterations, bitmap);

    arrfixed2d_free(&pixel_datas);
}

void sppm_free(SPPM* sppm){
    arr_free(&sppm->emitters.emitters);
    arr_free(&sppm->emitters.prefix_intensity);
}