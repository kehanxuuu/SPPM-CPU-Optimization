#include "intersection_l.h"

//    TODO: maybe add additional check before blending to see if there exists values with this type
//          for all blends (if statements)

void bsdf_sample_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 mesh_material = _mm256_load_ps(&isects->mesh_material.data[ind]);
    __m256 is_diffuse = _mm256_cmp_ps(_mm256_set1_ps(DIFFUSE), mesh_material, _CMP_EQ_OQ);
    __m256 is_specular = _mm256_cmp_ps(_mm256_set1_ps(SPECULAR), mesh_material, _CMP_EQ_OQ);
    __m256 is_dielectric = _mm256_cmp_ps(_mm256_set1_ps(DIELECTRIC), mesh_material, _CMP_EQ_OQ);

    __m256 samples0 = simd_rand_full();
    __m256 samples1 = simd_rand_full();

    __m256 diffuse_res_x, diffuse_res_y, diffuse_res_z;
    __m256 specular_res_x, specular_res_y, specular_res_z;
    __m256 dielectric_res_x, dielectric_res_y, dielectric_res_z;

    bsdf_sample_diffuse_l(isects, ind, samples0, samples1, &diffuse_res_x, &diffuse_res_y, &diffuse_res_z);
    bsdf_sample_specular_l(isects, ind, &specular_res_x, &specular_res_y, &specular_res_z);
    bsdf_sample_dielectric_l(isects, ind, samples0, &dielectric_res_x, &dielectric_res_y, &dielectric_res_z);

    __m256 t0, t1, t2;
    t0 = _mm256_blendv_ps(  diffuse_res_x, specular_res_x, is_specular);
    *res_x = _mm256_blendv_ps(t0, dielectric_res_x, is_dielectric);

    t1 = _mm256_blendv_ps(  diffuse_res_y, specular_res_y, is_specular);
    *res_y = _mm256_blendv_ps(t1, dielectric_res_y, is_dielectric);

    t2 = _mm256_blendv_ps(  diffuse_res_z, specular_res_z, is_specular);
    *res_z = _mm256_blendv_ps(t2, dielectric_res_z, is_dielectric);
}

void bsdf_eval_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 mesh_material = _mm256_load_ps(&isects->mesh_material.data[ind]);
    __m256 is_diffuse = _mm256_cmp_ps(_mm256_set1_ps(DIFFUSE), mesh_material, _CMP_EQ_OQ);
    __m256 is_specular = _mm256_cmp_ps(_mm256_set1_ps(SPECULAR), mesh_material, _CMP_EQ_OQ);
    __m256 is_dielectric = _mm256_cmp_ps(_mm256_set1_ps(DIELECTRIC), mesh_material, _CMP_EQ_OQ);

    __m256 diffuse_res_x, diffuse_res_y, diffuse_res_z;
    __m256 specular_res_x, specular_res_y, specular_res_z;
    __m256 dielectric_res_x, dielectric_res_y, dielectric_res_z;

    bsdf_eval_diffuse_l(isects, ind, &diffuse_res_x, &diffuse_res_y, &diffuse_res_z);
    bsdf_eval_specular_l(isects, ind, &specular_res_x, &specular_res_y, &specular_res_z);
    bsdf_eval_dielectric_l(isects, ind, &dielectric_res_x, &dielectric_res_y, &dielectric_res_z);

    __m256 t0, t1, t2;
    t0 = _mm256_blendv_ps(  diffuse_res_x, specular_res_x, is_specular);
    *res_x = _mm256_blendv_ps(t0, dielectric_res_x, is_dielectric);

    t1 = _mm256_blendv_ps(  diffuse_res_y, specular_res_y, is_specular);
    *res_y = _mm256_blendv_ps(t1, dielectric_res_y, is_dielectric);

    t2 = _mm256_blendv_ps(  diffuse_res_z, specular_res_z, is_specular);
    *res_z = _mm256_blendv_ps(t2, dielectric_res_z, is_dielectric);
}

void bsdf_sample_diffuse_l(struct IntersectionL* isects, size_t ind, __m256 sample0, __m256 sample1, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = _mm256_load_ps(&isects->wi.x[ind]);
    __m256 wi_y = _mm256_load_ps(&isects->wi.y[ind]);
    __m256 wi_z = _mm256_load_ps(&isects->wi.z[ind]);
    __m256 n_x = _mm256_load_ps(&isects->n.x[ind]);
    __m256 n_y = _mm256_load_ps(&isects->n.y[ind]);
    __m256 n_z = _mm256_load_ps(&isects->n.z[ind]);

    __m256 dot_res = vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z);

//    wo not used if false, so can set with arb value
    __m256 wo_x, wo_y, wo_z;
    vector3fl_square_to_cosine_hemisphere(sample0, sample1, n_x, n_y, n_z, &wo_x, &wo_y, &wo_z);
    _mm256_store_ps(&isects->wo.x[ind], wo_x);
    _mm256_store_ps(&isects->wo.y[ind], wo_y);
    _mm256_store_ps(&isects->wo.z[ind], wo_z);

    __m256 cmp0 = _mm256_cmp_ps(dot_res, _mm256_set1_ps(0.0f), _CMP_GE_OQ);
    __m256 albedo_x = _mm256_load_ps(&isects->mesh_albedo.x[ind]);
    __m256 albedo_y = _mm256_load_ps(&isects->mesh_albedo.y[ind]);
    __m256 albedo_z = _mm256_load_ps(&isects->mesh_albedo.z[ind]);
    *res_x = _mm256_blendv_ps(albedo_x, _mm256_set1_ps(0.0f), cmp0);
    *res_y = _mm256_blendv_ps(albedo_y, _mm256_set1_ps(0.0f), cmp0);
    *res_z = _mm256_blendv_ps(albedo_z, _mm256_set1_ps(0.0f), cmp0);
}

void bsdf_eval_diffuse_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = _mm256_load_ps(&isects->wi.x[ind]);
    __m256 wi_y = _mm256_load_ps(&isects->wi.y[ind]);
    __m256 wi_z = _mm256_load_ps(&isects->wi.z[ind]);
    __m256 n_x = _mm256_load_ps(&isects->n.x[ind]);
    __m256 n_y = _mm256_load_ps(&isects->n.y[ind]);
    __m256 n_z = _mm256_load_ps(&isects->n.z[ind]);
    __m256 wi_dot_res = vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z);

    __m256 wo_x = _mm256_load_ps(&isects->wo.x[ind]);
    __m256 wo_y = _mm256_load_ps(&isects->wo.y[ind]);
    __m256 wo_z = _mm256_load_ps(&isects->wo.z[ind]);
    __m256 wo_dot_res = vector3fl_dot(wo_x, wo_y, wo_z, n_x, n_y, n_z);

    __m256 cmp0 = _mm256_or_ps(_mm256_cmp_ps(wi_dot_res, _mm256_set1_ps(0.0f), _CMP_GE_OQ),
                               _mm256_cmp_ps(wo_dot_res, _mm256_set1_ps(0.0f), _CMP_LE_OQ));

    __m256 p_albedo_x = _mm256_mul_ps(_mm256_load_ps(&isects->mesh_albedo.x[ind]), _mm256_set1_ps(INV_PI));
    __m256 p_albedo_y = _mm256_mul_ps(_mm256_load_ps(&isects->mesh_albedo.y[ind]), _mm256_set1_ps(INV_PI));
    __m256 p_albedo_z = _mm256_mul_ps(_mm256_load_ps(&isects->mesh_albedo.z[ind]), _mm256_set1_ps(INV_PI));

    *res_x = _mm256_blendv_ps(p_albedo_x, _mm256_set1_ps(0.0f), cmp0);
    *res_y = _mm256_blendv_ps(p_albedo_y, _mm256_set1_ps(0.0f), cmp0);
    *res_z = _mm256_blendv_ps(p_albedo_z, _mm256_set1_ps(0.0f), cmp0);
}

void bsdf_sample_specular_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = _mm256_load_ps(&isects->wi.x[ind]);
    __m256 wi_y = _mm256_load_ps(&isects->wi.y[ind]);
    __m256 wi_z = _mm256_load_ps(&isects->wi.z[ind]);
    __m256 n_x = _mm256_load_ps(&isects->n.x[ind]);
    __m256 n_y = _mm256_load_ps(&isects->n.y[ind]);
    __m256 n_z = _mm256_load_ps(&isects->n.z[ind]);

    __m256 cos_wi_n = _mm256_neg_ps(vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z));
    __m256 cos_wi_n_2 = _mm256_mul_ps(_mm256_set1_ps(2.0f), cos_wi_n);
    __m256 scaled_n_2_x = _mm256_mul_ps(cos_wi_n_2, n_x);
    __m256 scaled_n_2_y = _mm256_mul_ps(cos_wi_n_2, n_y);
    __m256 scaled_n_2_z = _mm256_mul_ps(cos_wi_n_2, n_z);

    _mm256_store_ps(&isects->wo.x[ind], _mm256_add_ps(wi_x, scaled_n_2_x));
    _mm256_store_ps(&isects->wo.y[ind], _mm256_add_ps(wi_y, scaled_n_2_y));
    _mm256_store_ps(&isects->wo.z[ind], _mm256_add_ps(wi_z, scaled_n_2_z));

    *res_x = _mm256_load_ps(&isects->mesh_albedo.x[ind]);
    *res_y = _mm256_load_ps(&isects->mesh_albedo.y[ind]);
    *res_z = _mm256_load_ps(&isects->mesh_albedo.z[ind]);
}

void bsdf_eval_specular_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z){
    *res_x = _mm256_set1_ps(0.0f);
    *res_y = _mm256_set1_ps(0.0f);
    *res_z = _mm256_set1_ps(0.0f);
}

// TODO: redundant code cleanup and calculate chained if states bottom up
void bsdf_sample_dielectric_l(struct IntersectionL* isects, size_t ind, __m256 sample0, __m256* res_x, __m256* res_y, __m256* res_z){
    __m256 wi_x = _mm256_load_ps(&isects->wi.x[ind]);
    __m256 wi_y = _mm256_load_ps(&isects->wi.y[ind]);
    __m256 wi_z = _mm256_load_ps(&isects->wi.z[ind]);
    __m256 n_x = _mm256_load_ps(&isects->n.x[ind]);
    __m256 n_y = _mm256_load_ps(&isects->n.y[ind]);
    __m256 n_z = _mm256_load_ps(&isects->n.z[ind]);

    __m256 costheta1 = _mm256_neg_ps(vector3fl_dot(wi_x, wi_y, wi_z, n_x, n_y, n_z));

    __m256 is_interior = _mm256_load_ps(&isects->interior.data[ind]);
    __m256 ir = _mm256_load_ps(&isects->mesh_ir.data[ind]);
    __m256 n1 = _mm256_blendv_ps(_mm256_set1_ps(1.0f), ir, is_interior);
    __m256 n2 = _mm256_blendv_ps( ir, _mm256_set1_ps(1.0f),is_interior);
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

    __m256 sintheta2 = _mm256_mul_ps(_mm256_mul_ps(eta, eta), _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(costheta1, costheta1)));
    __m256 costheta2 = _mm256_sqrt_ps(_mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(sintheta2, sintheta2)));
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

    _mm256_store_ps(&isects->wo.x[ind], wo_x);
    _mm256_store_ps(&isects->wo.y[ind], wo_y);
    _mm256_store_ps(&isects->wo.z[ind], wo_z);

    *res_x = _mm256_load_ps(&isects->mesh_albedo.x[ind]);
    *res_y = _mm256_load_ps(&isects->mesh_albedo.y[ind]);
    *res_z = _mm256_load_ps(&isects->mesh_albedo.z[ind]);
}

void bsdf_eval_dielectric_l(struct IntersectionL* isects, size_t ind, __m256* res_x, __m256* res_y, __m256* res_z){
    *res_x = _mm256_set1_ps(0.0f);
    *res_y = _mm256_set1_ps(0.0f);
    *res_z = _mm256_set1_ps(0.0f);
}
