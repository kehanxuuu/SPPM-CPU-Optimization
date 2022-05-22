#include "scene.h"
#include "warping.h"

void scene_init(struct Scene *scene) {
    arr_init_pointer(&scene->meshes, 10, 0);
    scene->n_meshes = scene->n_emitters = 0;
    scene->emitters = NULL;
    scene->accum_probabilities = NULL;
}

void scene_add(struct Scene *scene, struct Mesh *mesh) {
    arr_add_pointer(&scene->meshes, &mesh);
    scene->n_meshes++;
}

struct Mesh *scene_get(const struct Scene *scene, size_t index) {
    struct Mesh *mesh = (struct Mesh *) arr_get_pointer(&scene->meshes, index);
    return mesh;
}

void scene_free(struct Scene *scene) {
    for (size_t i = 0; i < scene->n_meshes; ++i) {
        struct Mesh *mesh = scene_get(scene, i);
        mesh_free(mesh);
        free(mesh);
    }
    arr_free_pointer(&scene->meshes);
    free(scene->emitters);
    free(scene->accum_probabilities);
}

bool scene_intersect(const struct Scene *scene, Ray *ray, struct Intersection *isect) {
    bool do_intersect = false;
    for (size_t mesh_idx = 0; mesh_idx < scene->n_meshes; ++mesh_idx) {
        struct Mesh *mesh = scene_get(scene, mesh_idx);
        if (mesh_intersect(mesh, ray, isect)) {
            do_intersect = true;
        }
    }
    return do_intersect;
}

bool scene_do_intersect(const struct Scene *scene, const Ray *ray) {
    for (size_t mesh_idx = 0; mesh_idx < scene->n_meshes; ++mesh_idx) {
        struct Mesh *mesh = scene_get(scene, mesh_idx);
        if (mesh_do_intersect(mesh, ray)) {
            return true;
        }
    }
    return false;
}

void scene_finish(struct Scene *scene) {
    if (scene->emitters) free(scene->emitters);
    if (scene->accum_probabilities) free(scene->accum_probabilities);

    for (size_t i = 0; i < scene->n_meshes; ++i) {
        if (!vv_equal(&scene_get(scene, i)->emission, &ZERO_VEC)) scene->n_emitters++;
    }
    if (scene->n_emitters == 0) {
        fprintf(stderr, "Expect one or more emitters!\n");
        exit(1);
    }
    scene->emitters = (Mesh **) malloc(scene->n_emitters * sizeof(Mesh *));
    scene->accum_probabilities = (float *) malloc((scene->n_emitters + 1) * sizeof(float));

    float accum_area = 0;
    scene->accum_probabilities[0] = 0;
    for (size_t i = 0, j = 0; i < scene->n_meshes; ++i) {
        Mesh *mesh = scene_get(scene, i);
        if (!vv_equal(&mesh->emission, &ZERO_VEC)) {
            float surface_area = 0;
            switch (mesh->geometry->type) {
                case SPHERE: {
                    Sphere *s = (Sphere *) mesh->geometry->data;
                    surface_area = M_PI * s->r2;
                    break;
                }
                default:
                UNIMPLEMENTED;
            }
            accum_area += surface_area;
            scene->emitters[j] = mesh;
            scene->accum_probabilities[j + 1] = accum_area;
            ++j;
        }
    }
    // Normalize prob
    for (size_t i = 0; i <= scene->n_emitters; ++i) {
        scene->accum_probabilities[i] /= accum_area;
    }
}

struct Mesh *sample_emitter(const struct Scene *scene, float sample, float *pdf) {
    assert(scene->emitters && scene->accum_probabilities);
    for (size_t i = 0; i < scene->n_emitters; ++i) {
        if (scene->accum_probabilities[i] <= sample && sample < scene->accum_probabilities[i + 1]) {
            *pdf = scene->accum_probabilities[i + 1] - scene->accum_probabilities[i];
            return scene->emitters[i];
        }
    }
    return scene->emitters[0];
}

Vector estimate_direct_lighting(const struct Scene *scene, struct Intersection *isect) {
    float pdf;
    Mesh *emitter = sample_emitter(scene, randf(), &pdf);
    Ray shadow_ray;
    float G;  // geometric term
    switch (emitter->geometry->type) {
        case SPHERE: {
            Sphere *s = (Sphere *) emitter->geometry->data;
            // Sample the cone from the intersection point to the sphere silhouette
            Vector to_light = vv_sub(&s->c, &isect->p);
            vvs_fmaeq(&to_light, &isect->n, EPSILON);
            float dist = v_norm(&to_light);
            Vector light_dir = vs_div(&to_light, dist);
            float sin_theta_max = s->r / dist;
            float cos_theta_max = sqrtf(fmaxf(0, 1 - sin_theta_max * sin_theta_max));
            Vector dir = square_to_uniform_cone((Vector2f) {randf(), randf()}, cos_theta_max, &light_dir);
            float cos_theta = vv_dot(&dir, &light_dir);
            shadow_ray = (Ray) {
                    isect->p,
                    dir,
                    dist * cos_theta - sqrtf(s->r2 - dist * dist * (1 - cos_theta * cos_theta))
            };
            G = vv_dot(&isect->n, &shadow_ray.d);
            pdf /= 2 * M_PI * (1 - cos_theta_max);
            break;
        }
        default:
        UNIMPLEMENTED;
    }

    shadow_ray.o = ray_at(&shadow_ray, EPSILON);
    shadow_ray.t_max -= 4 * EPSILON;
    if (G <= 0 || scene_do_intersect(scene, &shadow_ray)) {
        // Shadowed by some obstacle
        return ZERO_VEC;
    }
    // Ld = Le * bsdf * G / pdf
    Vector Ld = emitter->emission;
    isect->wo = shadow_ray.d;
    Vector bsdf = bsdf_eval(isect);
    vv_muleq(&Ld, &bsdf);
    vs_muleq(&Ld, G / pdf);
    return Ld;
}
