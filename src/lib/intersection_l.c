#include "intersection_l.h"

void intersection_l_init(IntersectionL *isects, size_t size){
    floatl_init(&isects->mesh_material, size);
    vector3fl_init(&isects->mesh_albedo, size);
    vector3fl_init(&isects->mesh_emission, size);
    floatl_init(&isects->mesh_ir, size);
    vector3fl_init(&isects->p, size);
    vector3fl_init(&isects->n, size);
    vector3fl_init(&isects->wi, size);
    vector3fl_init(&isects->wo, size);
    floatl_init(&isects->interior, size);
}

void intersection_l_free(IntersectionL *isects){
    floatl_free(&isects->mesh_material);
    vector3fl_free(&isects->mesh_albedo);
    vector3fl_free(&isects->mesh_emission);
    floatl_free(&isects->mesh_ir);
    vector3fl_free(&isects->p);
    vector3fl_free(&isects->n);
    vector3fl_free(&isects->wi);
    vector3fl_free(&isects->wo);
    floatl_free(&isects->interior);
}

//    TODO: maybe add additional check before blending to see if there exists values with this type
//          for all blends (if statements)

void bsdf_sample_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 mesh_material = isects->mesh_material.data;
    // __m256 is_diffuse = _mm256_cmp_ps(_mm256_set1_ps(DIFFUSE), mesh_material, _CMP_EQ_OQ);
    __m256 is_specular = _mm256_cmp_ps(_mm256_set1_ps(SPECULAR), mesh_material, _CMP_EQ_OQ);
    __m256 is_dielectric = _mm256_cmp_ps(_mm256_set1_ps(DIELECTRIC), mesh_material, _CMP_EQ_OQ);

    __m256 samples0 = randf_full();
    __m256 samples1 = randf_full();

    __m256 diffuse_res_x, diffuse_res_y, diffuse_res_z;
    __m256 specular_res_x, specular_res_y, specular_res_z;
    __m256 dielectric_res_x, dielectric_res_y, dielectric_res_z;

    __m256 diff_wo_x, diff_wo_y, diff_wo_z, spec_wo_x, spec_wo_y, spec_wo_z;
    bsdf_sample_diffuse_m(isects, samples0, samples1, &diffuse_res_x, &diffuse_res_y, &diffuse_res_z);
    diff_wo_x = isects->wo.x;
    diff_wo_y = isects->wo.y;
    diff_wo_z = isects->wo.z;
    bsdf_sample_specular_m(isects, &specular_res_x, &specular_res_y, &specular_res_z);
    spec_wo_x = isects->wo.x;
    spec_wo_y = isects->wo.y;
    spec_wo_z = isects->wo.z;
    bsdf_sample_dielectric_m(isects, samples0, &dielectric_res_x, &dielectric_res_y, &dielectric_res_z);

    __m256 t0, t1, t2, t3, t4, t5;
    t0 = _mm256_blendv_ps(diffuse_res_x, specular_res_x, is_specular);
    *res_x = _mm256_blendv_ps(t0, dielectric_res_x, is_dielectric);

    t1 = _mm256_blendv_ps(diffuse_res_y, specular_res_y, is_specular);
    *res_y = _mm256_blendv_ps(t1, dielectric_res_y, is_dielectric);

    t2 = _mm256_blendv_ps(diffuse_res_z, specular_res_z, is_specular);
    *res_z = _mm256_blendv_ps(t2, dielectric_res_z, is_dielectric);

    t3 = _mm256_blendv_ps(diff_wo_x, spec_wo_x, is_specular);
    isects->wo.x = _mm256_blendv_ps(t3, isects->wo.x, is_dielectric);

    t4 = _mm256_blendv_ps(diff_wo_y, spec_wo_y, is_specular);
    isects->wo.y = _mm256_blendv_ps(t4, isects->wo.y, is_dielectric);

    t5 = _mm256_blendv_ps(diff_wo_z, spec_wo_z, is_specular);
    isects->wo.z = _mm256_blendv_ps(t5, isects->wo.z, is_dielectric);
}

void bsdf_eval_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 mesh_material = isects->mesh_material.data;
    // __m256 is_diffuse = _mm256_cmp_ps(_mm256_set1_ps(DIFFUSE), mesh_material, _CMP_EQ_OQ);
    __m256 is_specular = _mm256_cmp_ps(_mm256_set1_ps(SPECULAR), mesh_material, _CMP_EQ_OQ);
    __m256 is_dielectric = _mm256_cmp_ps(_mm256_set1_ps(DIELECTRIC), mesh_material, _CMP_EQ_OQ);

    __m256 diffuse_res_x, diffuse_res_y, diffuse_res_z;
    __m256 specular_res_x, specular_res_y, specular_res_z;
    __m256 dielectric_res_x, dielectric_res_y, dielectric_res_z;

    bsdf_eval_diffuse_m(isects, &diffuse_res_x, &diffuse_res_y, &diffuse_res_z);
    bsdf_eval_specular_m(isects, &specular_res_x, &specular_res_y, &specular_res_z);
    bsdf_eval_dielectric_m(isects, &dielectric_res_x, &dielectric_res_y, &dielectric_res_z);

    __m256 t0, t1, t2;
    t0 = _mm256_blendv_ps(diffuse_res_x, specular_res_x, is_specular);
    *res_x = _mm256_blendv_ps(t0, dielectric_res_x, is_dielectric);

    t1 = _mm256_blendv_ps(diffuse_res_y, specular_res_y, is_specular);
    *res_y = _mm256_blendv_ps(t1, dielectric_res_y, is_dielectric);

    t2 = _mm256_blendv_ps(diffuse_res_z, specular_res_z, is_specular);
    *res_z = _mm256_blendv_ps(t2, dielectric_res_z, is_dielectric);
}

void bsdf_sample_diffuse_m(IntersectionM* isects, __m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = isects->wi.x;
    __m256 wi_y = isects->wi.y;
    __m256 wi_z = isects->wi.z;
    __m256 n_x = isects->n.x;
    __m256 n_y = isects->n.y;
    __m256 n_z = isects->n.z;

    __m256 dot_res = vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z);

//    wo not used if false, so can set with arb value
    __m256 wo_x, wo_y, wo_z;
    vector3fl_square_to_cosine_hemisphere(sample0, sample1, n_x, n_y, n_z, &wo_x, &wo_y, &wo_z);
    isects->wo.x = wo_x;
    isects->wo.y = wo_y;
    isects->wo.z = wo_z;

    __m256 Zero = _mm256_setzero_ps();
    __m256 cmp0 = _mm256_cmp_ps(dot_res, Zero, _CMP_GE_OQ);
    __m256 albedo_x = isects->mesh_albedo.x;
    __m256 albedo_y = isects->mesh_albedo.y;
    __m256 albedo_z = isects->mesh_albedo.z;
    *res_x = _mm256_blendv_ps(albedo_x, Zero, cmp0);
    *res_y = _mm256_blendv_ps(albedo_y, Zero, cmp0);
    *res_z = _mm256_blendv_ps(albedo_z, Zero, cmp0);
}

void bsdf_eval_diffuse_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = isects->wi.x;
    __m256 wi_y = isects->wi.y;
    __m256 wi_z = isects->wi.z;
    __m256 n_x = isects->n.x;
    __m256 n_y = isects->n.y;
    __m256 n_z = isects->n.z;
    __m256 wi_dot_res = vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z);

    __m256 wo_x = isects->wo.x;
    __m256 wo_y = isects->wo.y;
    __m256 wo_z = isects->wo.z;
    __m256 wo_dot_res = vector3fl_dot(wo_x, wo_y, wo_z, n_x, n_y, n_z);

    __m256 Zero = _mm256_setzero_ps();
    __m256 Inv_Pi = _mm256_set1_ps(INV_PI);
    __m256 cmp0 = _mm256_or_ps(_mm256_cmp_ps(wi_dot_res, Zero, _CMP_GE_OQ),
                               _mm256_cmp_ps(wo_dot_res, Zero, _CMP_LE_OQ));

    __m256 p_albedo_x = _mm256_mul_ps(isects->mesh_albedo.x, Inv_Pi);
    __m256 p_albedo_y = _mm256_mul_ps(isects->mesh_albedo.y, Inv_Pi);
    __m256 p_albedo_z = _mm256_mul_ps(isects->mesh_albedo.z, Inv_Pi);

    *res_x = _mm256_blendv_ps(p_albedo_x, Zero, cmp0);
    *res_y = _mm256_blendv_ps(p_albedo_y, Zero, cmp0);
    *res_z = _mm256_blendv_ps(p_albedo_z, Zero, cmp0);
}

void bsdf_sample_specular_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = isects->wi.x;
    __m256 wi_y = isects->wi.y;
    __m256 wi_z = isects->wi.z;
    __m256 n_x = isects->n.x;
    __m256 n_y = isects->n.y;
    __m256 n_z = isects->n.z;

    __m256 neg_cos_wi_n = vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z);
    __m256 neg_cos_wi_n_2 = _mm256_mul_ps(_mm256_set1_ps(2.0f), neg_cos_wi_n);
    __m256 neg_scaled_n_2_x = _mm256_mul_ps(neg_cos_wi_n_2, n_x);
    __m256 neg_scaled_n_2_y = _mm256_mul_ps(neg_cos_wi_n_2, n_y);
    __m256 neg_scaled_n_2_z = _mm256_mul_ps(neg_cos_wi_n_2, n_z);

    isects->wo.x = _mm256_sub_ps(wi_x, neg_scaled_n_2_x);
    isects->wo.y = _mm256_sub_ps(wi_y, neg_scaled_n_2_y);
    isects->wo.z = _mm256_sub_ps(wi_z, neg_scaled_n_2_z);

    *res_x = isects->mesh_albedo.x;
    *res_y = isects->mesh_albedo.y;
    *res_z = isects->mesh_albedo.z;
}

void bsdf_eval_specular_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 Zero = _mm256_setzero_ps();
    *res_x = Zero;
    *res_y = Zero;
    *res_z = Zero;
}

// TODO: redundant code cleanup and calculate chained if states bottom up
void bsdf_sample_dielectric_m(IntersectionM* isects, __m256 sample0, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = isects->wi.x;
    __m256 wi_y = isects->wi.y;
    __m256 wi_z = isects->wi.z;
    __m256 n_x = isects->n.x;
    __m256 n_y = isects->n.y;
    __m256 n_z = isects->n.z;

    __m256 costheta1 = _mm256_neg_ps(vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z));

    __m256 One = _mm256_set1_ps(1.0f);
    __m256 is_interior = isects->interior.data;
    __m256 ir = isects->mesh_ir.data;
    __m256 n1 = _mm256_blendv_ps(One, ir, is_interior);
    __m256 n2 = _mm256_blendv_ps(ir, One, is_interior);
    __m256 fresnel_term = vector3fl_fresnel(costheta1, n1, n2);

    __m256 cmp0 = _mm256_cmp_ps(sample0, fresnel_term, _CMP_LT_OQ);
    __m256 t0 = _mm256_mul_ps(_mm256_set1_ps(2.0f), costheta1);
    __m256 wo_x = _mm256_fmadd_ps(t0, n_x, wi_x);
    __m256 wo_y = _mm256_fmadd_ps(t0, n_y, wi_y);
    __m256 wo_z = _mm256_fmadd_ps(t0, n_z, wi_z);

    __m256 eta = _mm256_div_ps(n1, n2);
    __m256 tangent_x = _mm256_fmadd_ps(costheta1, n_x, wi_x);
    __m256 tangent_y = _mm256_fmadd_ps(costheta1, n_y, wi_y);
    __m256 tangent_z = _mm256_fmadd_ps(costheta1, n_z, wi_z);

    __m256 cmp1 = vector3fl_is_zero(tangent_x, tangent_y, tangent_z);
    __m256 cmp01 = _mm256_andnot_ps(cmp0,  cmp1);
    wo_x = _mm256_blendv_ps(wo_x, wi_x, cmp01);
    wo_y = _mm256_blendv_ps(wo_y, wi_y, cmp01);
    wo_z = _mm256_blendv_ps(wo_z, wi_z, cmp01);

    __m256 sintheta2 = _mm256_mul_ps(_mm256_mul_ps(eta, eta), _mm256_sub_ps(One, _mm256_mul_ps(costheta1, costheta1)));
    __m256 costheta2 = _mm256_sqrt_ps(_mm256_sub_ps(One, _mm256_mul_ps(sintheta2, sintheta2)));
    __m256 n_t_x, n_t_y, n_t_z;
    vector3fl_normalize(tangent_x, tangent_y, tangent_z, &n_t_x, &n_t_y, &n_t_z);
    n_t_x = _mm256_mul_ps(n_t_x, sintheta2);
    n_t_y = _mm256_mul_ps(n_t_y, sintheta2);
    n_t_z = _mm256_mul_ps(n_t_z, sintheta2);
    __m256 t1, t2, t3, t4;
    t1 = _mm256_neg_ps(costheta2);
    t2 = _mm256_fmadd_ps(t1, n_x, n_t_x);
    t3 = _mm256_fmadd_ps(t1, n_y, n_t_y);
    t4 = _mm256_fmadd_ps(t1, n_z, n_t_z);

    wo_x = _mm256_blendv_ps( t2, wo_x,_mm256_or_ps(cmp0, cmp01));
    wo_y = _mm256_blendv_ps( t3, wo_y,_mm256_or_ps(cmp0, cmp01));
    wo_z = _mm256_blendv_ps( t4, wo_z,_mm256_or_ps(cmp0, cmp01));

    isects->wo.x = wo_x;
    isects->wo.y = wo_y;
    isects->wo.z = wo_z;

    *res_x = isects->mesh_albedo.x;
    *res_y = isects->mesh_albedo.y;
    *res_z = isects->mesh_albedo.z;
}

void bsdf_eval_dielectric_m(IntersectionM* isects, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 Zero = _mm256_setzero_ps();
    *res_x = Zero;
    *res_y = Zero;
    *res_z = Zero;
}

__m256 scene_intersect_m(struct Scene *scene, __m256 ray_o_x, __m256 ray_o_y, __m256 ray_o_z, __m256 ray_d_x, __m256 ray_d_y, __m256 ray_d_z, __m256* ray_t_max, IntersectionM* isect){
    __m256 do_intersect = _mm256_setzero_ps();
    for (int mesh_idx = 0; mesh_idx < scene->n_meshes; ++mesh_idx) {
        struct Mesh *mesh = scene_get(scene, mesh_idx);
        struct Geometry *geometry = mesh->geometry;
//      only spheres
        struct Sphere *sphere = (struct Sphere *) geometry->data;
        __m256 sphere_c_x = _mm256_set1_ps(sphere->c.x);
        __m256 sphere_c_y = _mm256_set1_ps(sphere->c.y);
        __m256 sphere_c_z = _mm256_set1_ps(sphere->c.z);
        __m256 sphere_r2 = _mm256_set1_ps(sphere->r2);

        __m256 oc_x = _mm256_sub_ps(ray_o_x, sphere_c_x);
        __m256 oc_y = _mm256_sub_ps(ray_o_y, sphere_c_y);
        __m256 oc_z = _mm256_sub_ps(ray_o_z, sphere_c_z);

        __m256 a = vector3fl_dot(ray_d_x, ray_d_y, ray_d_z, ray_d_x, ray_d_y, ray_d_z);
        __m256 half_b = vector3fl_dot(ray_d_x, ray_d_y, ray_d_z, oc_x, oc_y, oc_z);
        __m256 c = _mm256_sub_ps(vector3fl_dot(oc_x, oc_y, oc_z, oc_x, oc_y, oc_z), sphere_r2);
        __m256 discriminant = _mm256_sub_ps(_mm256_mul_ps(half_b, half_b), _mm256_mul_ps(a, c));

        __m256 cur_do_intersect = _mm256_cmp_ps(discriminant, _mm256_setzero_ps(), _CMP_GE_OQ);
        __m256 sqrt_d = _mm256_sqrt_ps(discriminant);
        __m256 root_small = _mm256_div_ps(_mm256_sub_ps(_mm256_neg_ps(half_b), sqrt_d), a);
        __m256 root_big = _mm256_div_ps(_mm256_add_ps(_mm256_neg_ps(half_b), sqrt_d), a);
        __m256 is_interior = _mm256_cmp_ps(root_small, _mm256_setzero_ps(), _CMP_LT_OQ);
        __m256 root = _mm256_blendv_ps(root_small, root_big, is_interior);

        cur_do_intersect = _mm256_and_ps(cur_do_intersect, _mm256_cmp_ps(root, *ray_t_max, _CMP_LE_OQ));
        cur_do_intersect = _mm256_and_ps(cur_do_intersect, _mm256_cmp_ps(root, _mm256_setzero_ps(), _CMP_GT_OQ));

        do_intersect = _mm256_or_ps(do_intersect, cur_do_intersect);

        *ray_t_max = _mm256_blendv_ps(*ray_t_max, root, cur_do_intersect);
        isect->mesh_material.data = _mm256_blendv_ps(isect->mesh_material.data, _mm256_set1_ps(mesh->material),
                                                     cur_do_intersect);
        isect->mesh_albedo.x = _mm256_blendv_ps(isect->mesh_albedo.x, _mm256_set1_ps(mesh->albedo.x), cur_do_intersect);
        isect->mesh_albedo.y = _mm256_blendv_ps(isect->mesh_albedo.y, _mm256_set1_ps(mesh->albedo.y), cur_do_intersect);
        isect->mesh_albedo.z = _mm256_blendv_ps(isect->mesh_albedo.z, _mm256_set1_ps(mesh->albedo.z), cur_do_intersect);
        isect->mesh_emission.x = _mm256_blendv_ps(isect->mesh_emission.x, _mm256_set1_ps(mesh->emission.x),
                                                  cur_do_intersect);
        isect->mesh_emission.y = _mm256_blendv_ps(isect->mesh_emission.y, _mm256_set1_ps(mesh->emission.y),
                                                  cur_do_intersect);
        isect->mesh_emission.z = _mm256_blendv_ps(isect->mesh_emission.z, _mm256_set1_ps(mesh->emission.z),
                                                  cur_do_intersect);
        isect->mesh_ir.data = _mm256_blendv_ps(isect->mesh_ir.data, _mm256_set1_ps(mesh->ir), cur_do_intersect);

        isect->p.x = _mm256_blendv_ps(isect->p.x, _mm256_fmadd_ps(root, ray_d_x, ray_o_x), cur_do_intersect);
        isect->p.y = _mm256_blendv_ps(isect->p.y, _mm256_fmadd_ps(root, ray_d_y, ray_o_y), cur_do_intersect);
        isect->p.z = _mm256_blendv_ps(isect->p.z, _mm256_fmadd_ps(root, ray_d_z, ray_o_z), cur_do_intersect);
        isect->interior.data = _mm256_blendv_ps(isect->interior.data, is_interior, cur_do_intersect);
        __m256 un_isect_n_x = _mm256_sub_ps(isect->p.x, sphere_c_x);
        __m256 un_isect_n_y = _mm256_sub_ps(isect->p.y, sphere_c_y);
        __m256 un_isect_n_z = _mm256_sub_ps(isect->p.z, sphere_c_z);
        __m256 isect_n_x, isect_n_y, isect_n_z;
        vector3fl_normalize(un_isect_n_x, un_isect_n_y, un_isect_n_z, &isect_n_x, &isect_n_y, &isect_n_z);
        isect_n_x = _mm256_blendv_ps(isect_n_x, _mm256_neg_ps(isect_n_x), is_interior);
        isect_n_y = _mm256_blendv_ps(isect_n_y, _mm256_neg_ps(isect_n_y), is_interior);
        isect_n_z = _mm256_blendv_ps(isect_n_z, _mm256_neg_ps(isect_n_z), is_interior);
        isect->n.x = _mm256_blendv_ps(isect->n.x, isect_n_x, cur_do_intersect);
        isect->n.y = _mm256_blendv_ps(isect->n.y, isect_n_y, cur_do_intersect);
        isect->n.z = _mm256_blendv_ps(isect->n.z, isect_n_z, cur_do_intersect);

        __m256 n_wi_x, n_wi_y, n_wi_z;
        vector3fl_normalize(ray_d_x, ray_d_y, ray_d_z, &n_wi_x, &n_wi_y, &n_wi_z);

        isect->wi.x = _mm256_blendv_ps(isect->wi.x, n_wi_x, cur_do_intersect);
        isect->wi.y = _mm256_blendv_ps(isect->wi.y, n_wi_y, cur_do_intersect);
        isect->wi.z = _mm256_blendv_ps(isect->wi.z, n_wi_z, cur_do_intersect);
    }
    return do_intersect;
}

__m256 scene_do_intersect_m(struct Scene *scene, __m256 ray_o_x, __m256 ray_o_y, __m256 ray_o_z, __m256 ray_d_x, __m256 ray_d_y, __m256 ray_d_z, __m256 ray_t_max){
    __m256 do_intersect = _mm256_setzero_ps();
    for (int mesh_idx = 0; mesh_idx < scene->n_meshes; ++mesh_idx) {
        struct Mesh *mesh = scene_get(scene, mesh_idx);
        struct Geometry *geometry = mesh->geometry;
//      only spheres
        struct Sphere *sphere = (struct Sphere *) geometry->data;
        __m256 sphere_c_x = _mm256_set1_ps(sphere->c.x);
        __m256 sphere_c_y = _mm256_set1_ps(sphere->c.y);
        __m256 sphere_c_z = _mm256_set1_ps(sphere->c.z);
        __m256 sphere_r2 = _mm256_set1_ps(sphere->r2);

        __m256 oc_x = _mm256_sub_ps(ray_o_x, sphere_c_x);
        __m256 oc_y = _mm256_sub_ps(ray_o_y, sphere_c_y);
        __m256 oc_z = _mm256_sub_ps(ray_o_z, sphere_c_z);

        __m256 a = vector3fl_dot(ray_d_x, ray_d_y, ray_d_z, ray_d_x, ray_d_y, ray_d_z);
        __m256 half_b = vector3fl_dot(ray_d_x, ray_d_y, ray_d_z, oc_x, oc_y, oc_z);
        __m256 c = _mm256_sub_ps(vector3fl_dot(oc_x, oc_y, oc_z, oc_x, oc_y, oc_z), sphere_r2);
        __m256 discriminant = _mm256_sub_ps(_mm256_mul_ps(half_b, half_b), _mm256_mul_ps(a, c));

        __m256 cur_do_intersect = _mm256_cmp_ps(discriminant, _mm256_setzero_ps(), _CMP_GE_OQ);
        __m256 sqrt_d = _mm256_sqrt_ps(discriminant);
        __m256 root_small = _mm256_div_ps(_mm256_sub_ps(_mm256_neg_ps(half_b), sqrt_d), a);
        __m256 root_big = _mm256_div_ps(_mm256_add_ps(_mm256_neg_ps(half_b), sqrt_d), a);
        __m256 is_interior = _mm256_cmp_ps(root_small, _mm256_setzero_ps(), _CMP_LT_OQ);
        __m256 root = _mm256_blendv_ps(root_small, root_big, is_interior);

        cur_do_intersect = _mm256_and_ps(cur_do_intersect, _mm256_cmp_ps(root, ray_t_max, _CMP_LE_OQ));
        cur_do_intersect = _mm256_and_ps(cur_do_intersect, _mm256_cmp_ps(root, _mm256_setzero_ps(), _CMP_GT_OQ));

        do_intersect = _mm256_or_ps(do_intersect, cur_do_intersect);
    }
    return do_intersect;
}

void estimate_direct_lighting_m(struct Scene *scene, IntersectionM* isect, __m256* res_x, __m256* res_y, __m256* res_z) {
    __m256 pdf = _mm256_set1_ps(scene->accum_probabilities[1]);
    __m256i emitter_id = _mm256_setzero_si256();
    __m256 sample = randf_full();

    for (int i = 0; i < scene->n_emitters; ++i) {
        __m256 ac_prob_i = _mm256_set1_ps(scene->accum_probabilities[i]);
        __m256 ac_prob_i1 = _mm256_set1_ps(scene->accum_probabilities[i + 1]);
        __m256 cmp0 = _mm256_and_ps(_mm256_cmp_ps(ac_prob_i, sample, _CMP_LE_OQ),
                                    _mm256_cmp_ps(sample, ac_prob_i1, _CMP_LT_OQ));
        pdf = _mm256_blendv_ps(pdf, _mm256_sub_ps(ac_prob_i1, ac_prob_i), cmp0);
        __m256i mask_i = cmp0; // _mm256_cvtps_epi32(cmp0) does't work
        emitter_id = _mm256_blendv_epi8(emitter_id, _mm256_set1_epi32(i), mask_i);
    }
//  only spheres
    int emitter_id_impl[NUM_FLOAT_SIMD] __attribute__((__aligned__(64)));
    _mm256_store_si256((__m256i *) emitter_id_impl, emitter_id);
    Sphere *spheres[NUM_FLOAT_SIMD];
    for (int i = 0; i < NUM_FLOAT_SIMD; i++) {
        spheres[i] = scene->emitters[emitter_id_impl[i]]->geometry->data;
    }

    __m256 sphere_c_x = _mm256_setr_ps(spheres[0]->c.x, spheres[1]->c.x, spheres[2]->c.x, spheres[3]->c.x,
                                       spheres[4]->c.x, spheres[5]->c.x, spheres[6]->c.x, spheres[7]->c.x);
    __m256 sphere_c_y = _mm256_setr_ps(spheres[0]->c.y, spheres[1]->c.y, spheres[2]->c.y, spheres[3]->c.y,
                                       spheres[4]->c.y, spheres[5]->c.y, spheres[6]->c.y, spheres[7]->c.y);
    __m256 sphere_c_z = _mm256_setr_ps(spheres[0]->c.z, spheres[1]->c.z, spheres[2]->c.z, spheres[3]->c.z,
                                       spheres[4]->c.z, spheres[5]->c.z, spheres[6]->c.z, spheres[7]->c.z);

    __m256 to_light_x = _mm256_fmadd_ps(isect->n.x, _mm256_set1_ps(EPSILON), _mm256_sub_ps(sphere_c_x, isect->p.x));
    __m256 to_light_y = _mm256_fmadd_ps(isect->n.y, _mm256_set1_ps(EPSILON), _mm256_sub_ps(sphere_c_y, isect->p.y));
    __m256 to_light_z = _mm256_fmadd_ps(isect->n.z, _mm256_set1_ps(EPSILON), _mm256_sub_ps(sphere_c_z, isect->p.z));

    // fix precision problem of vector3fl_rnorm
    __m256 dist = vector3fl_norm(to_light_x, to_light_y, to_light_z);
    __m256 rdist = _mm256_div_ps(_mm256_set1_ps(1.0), dist);

    __m256 light_dir_x = _mm256_mul_ps(to_light_x, rdist);
    __m256 light_dir_y = _mm256_mul_ps(to_light_y, rdist);
    __m256 light_dir_z = _mm256_mul_ps(to_light_z, rdist);

    __m256 sphere_r = _mm256_setr_ps(spheres[0]->r, spheres[1]->r, spheres[2]->r, spheres[3]->r,
                                     spheres[4]->r, spheres[5]->r, spheres[6]->r, spheres[7]->r);

    __m256 One = _mm256_set1_ps(1.0f);
    __m256 sin_theta_max = _mm256_mul_ps(sphere_r, rdist);
    __m256 cos_theta_max = _mm256_sqrt_ps(_mm256_max_ps(_mm256_setzero_ps(),
                                                        _mm256_sub_ps(One,
                                                                      _mm256_mul_ps(sin_theta_max, sin_theta_max))));

    __m256 dir_x, dir_y, dir_z;
    vector3fl_square_to_uniform_cone(randf_full(), randf_full(), cos_theta_max, light_dir_x, light_dir_y, light_dir_z,
                                     &dir_x, &dir_y, &dir_z);
    __m256 cos_theta = vector3fl_dot(dir_x, dir_y, dir_z, light_dir_x, light_dir_y, light_dir_z);

    __m256 shadow_ray_o_x = isect->p.x;
    __m256 shadow_ray_o_y = isect->p.y;
    __m256 shadow_ray_o_z = isect->p.z;
    __m256 shadow_ray_d_x = dir_x;
    __m256 shadow_ray_d_y = dir_y;
    __m256 shadow_ray_d_z = dir_z;
    __m256 t0 = _mm256_mul_ps(dist, cos_theta);
    __m256 t1 = _mm256_sub_ps(One, _mm256_mul_ps(cos_theta, cos_theta));
    __m256 t2 = _mm256_sqrt_ps(
            _mm256_sub_ps(_mm256_mul_ps(sphere_r, sphere_r), _mm256_mul_ps(_mm256_mul_ps(dist, dist), t1)));
    __m256 shadow_ray_t_max = _mm256_sub_ps(t0, t2);

    __m256 G = vector3fl_dot(isect->n.x, isect->n.y, isect->n.z, shadow_ray_d_x, shadow_ray_d_y, shadow_ray_d_z);
    pdf = _mm256_div_ps(pdf, _mm256_mul_ps(_mm256_set1_ps(M_2PI), _mm256_sub_ps(One, cos_theta_max)));

    __m256 epsilon = _mm256_set1_ps(EPSILON);
    shadow_ray_o_x = _mm256_fmadd_ps(shadow_ray_d_x, epsilon, shadow_ray_o_x);
    shadow_ray_o_y = _mm256_fmadd_ps(shadow_ray_d_y, epsilon, shadow_ray_o_y);
    shadow_ray_o_z = _mm256_fmadd_ps(shadow_ray_d_z, epsilon, shadow_ray_o_z);
    shadow_ray_t_max = _mm256_sub_ps(shadow_ray_t_max, _mm256_set1_ps(4 * EPSILON));

    __m256 do_intersect = scene_do_intersect_m(scene, shadow_ray_o_x, shadow_ray_o_y, shadow_ray_o_z, shadow_ray_d_x,
                                               shadow_ray_d_y, shadow_ray_d_z, shadow_ray_t_max);
    __m256 cmp0 = _mm256_cmp_ps(G, _mm256_setzero_ps(), _CMP_LE_OQ);
    __m256 cmp1 = _mm256_or_ps(do_intersect, cmp0);

    isect->wo.x = _mm256_blendv_ps(shadow_ray_d_x, isect->wo.x, cmp1);
    isect->wo.y = _mm256_blendv_ps(shadow_ray_d_y, isect->wo.y, cmp1);
    isect->wo.z = _mm256_blendv_ps(shadow_ray_d_z, isect->wo.z, cmp1);

    __m256 Ld_x = _mm256_setr_ps(scene->emitters[emitter_id_impl[0]]->emission.x,
                                 scene->emitters[emitter_id_impl[1]]->emission.x,
                                 scene->emitters[emitter_id_impl[2]]->emission.x,
                                 scene->emitters[emitter_id_impl[3]]->emission.x,
                                 scene->emitters[emitter_id_impl[4]]->emission.x,
                                 scene->emitters[emitter_id_impl[5]]->emission.x,
                                 scene->emitters[emitter_id_impl[6]]->emission.x,
                                 scene->emitters[emitter_id_impl[7]]->emission.x);
    __m256 Ld_y = _mm256_setr_ps(scene->emitters[emitter_id_impl[0]]->emission.y,
                                 scene->emitters[emitter_id_impl[1]]->emission.y,
                                 scene->emitters[emitter_id_impl[2]]->emission.y,
                                 scene->emitters[emitter_id_impl[3]]->emission.y,
                                 scene->emitters[emitter_id_impl[4]]->emission.y,
                                 scene->emitters[emitter_id_impl[5]]->emission.y,
                                 scene->emitters[emitter_id_impl[6]]->emission.y,
                                 scene->emitters[emitter_id_impl[7]]->emission.y);
    __m256 Ld_z = _mm256_setr_ps(scene->emitters[emitter_id_impl[0]]->emission.z,
                                 scene->emitters[emitter_id_impl[1]]->emission.z,
                                 scene->emitters[emitter_id_impl[2]]->emission.z,
                                 scene->emitters[emitter_id_impl[3]]->emission.z,
                                 scene->emitters[emitter_id_impl[4]]->emission.z,
                                 scene->emitters[emitter_id_impl[5]]->emission.z,
                                 scene->emitters[emitter_id_impl[6]]->emission.z,
                                 scene->emitters[emitter_id_impl[7]]->emission.z);
    __m256 bsdf_x, bsdf_y, bsdf_z;
    bsdf_eval_m(isect, &bsdf_x, &bsdf_y, &bsdf_z);

    __m256 t3 = _mm256_div_ps(G, pdf);
    Ld_x = _mm256_mul_ps(_mm256_mul_ps(Ld_x, bsdf_x), t3);
    Ld_y = _mm256_mul_ps(_mm256_mul_ps(Ld_y, bsdf_y), t3);
    Ld_z = _mm256_mul_ps(_mm256_mul_ps(Ld_z, bsdf_z), t3);

    *res_x = _mm256_blendv_ps(Ld_x, _mm256_setzero_ps(), cmp1);
    *res_y = _mm256_blendv_ps(Ld_y, _mm256_setzero_ps(), cmp1);
    *res_z = _mm256_blendv_ps(Ld_z, _mm256_setzero_ps(), cmp1);
}