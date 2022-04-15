#include "scene.h"

void scene_initialize(struct Scene *scene) {
    Array meshes_init = {0, 0, 0, NULL};
    scene->meshes = meshes_init;
}

void scene_initialize_with_mesh(struct Scene *scene, struct Mesh *mesh) {
    Array meshes_init = {0, 0, 0, NULL};
    scene->meshes = meshes_init;
    arr_init(&(scene->meshes), 50, 0, sizeof(struct Mesh *));
    arr_add(&(scene->meshes), &mesh);
}

void scene_add(struct Scene *scene, struct Mesh *mesh) {
    arr_add(&(scene->meshes), &mesh);
}

struct Mesh *scene_get(const struct Scene *scene, size_t index) {
    struct Mesh *mesh = *(struct Mesh **)arr_get(&(scene->meshes), index);
    return mesh;
}

size_t scene_size(const struct Scene *scene) {
    return arr_size(&(scene->meshes));
}

bool scene_intersect(struct Scene *scene, Ray *ray, struct Intersection *isect) {
    size_t mesh_num = scene_size(scene);
    Ray *ray_test = ray;
    float t_max_current = FLT_MAX; // or = ray->t_max here & ray->t_max = FLT_MAX at ray initialization?
    bool do_intersect = false;
    for (size_t mesh_idx = 0; mesh_idx < mesh_num; ++mesh_idx) {
        struct Mesh * mesh = scene_get(scene, mesh_idx);
        if (mesh_do_intersect(mesh, ray_test) && ray_test->t_max < t_max_current) {
            mesh_intersect(mesh, ray, isect);
            t_max_current = ray->t_max;
            do_intersect = true;
        }
    }
    return do_intersect;
}

bool scene_do_intersect(const struct Scene *scene, const Ray *ray) {
    size_t mesh_num = scene_size(scene);
    Ray *ray_test = ray;
    float t_max_current = FLT_MAX;
    bool do_intersect = false;
    for (size_t mesh_idx = 0; mesh_idx < mesh_num; ++mesh_idx) {
        struct Mesh *mesh = scene_get(scene, mesh_idx);
        struct Geometry g = (struct Geometry)(mesh->geometry);
        struct Sphere *s = (struct Sphere *)(mesh->geometry.data);
        if (mesh_do_intersect(mesh, ray_test)) {
            do_intersect = true;
            break;
        }
    }
    return do_intersect;
}