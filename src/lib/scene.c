#include "scene.h"

void scene_init(struct Scene *scene) {
    arr_init(&scene->meshes, 10, 0, sizeof(struct Mesh *));
}

void scene_add(struct Scene *scene, struct Mesh *mesh) {
    arr_add(&scene->meshes, &mesh);
}

struct Mesh *scene_get(const struct Scene *scene, size_t index) {
    struct Mesh *mesh = *(struct Mesh **)arr_get(&scene->meshes, index);
    return mesh;
}

size_t scene_size(const struct Scene *scene) {
    return arr_size(&(scene->meshes));
}

void scene_free(struct Scene *scene) {
    size_t number_of_meshes = scene_size(scene);
    for (size_t i = 0; i < number_of_meshes; ++i) {
        struct Mesh *mesh = scene_get(scene, i);
        mesh_free(mesh);
    }
    arr_free(&scene->meshes);
}

bool scene_intersect(const struct Scene *scene, Ray *ray, struct Intersection *isect) {
    size_t mesh_num = scene_size(scene);
    bool do_intersect = false;
    for (size_t mesh_idx = 0; mesh_idx < mesh_num; ++mesh_idx) {
        struct Mesh * mesh = scene_get(scene, mesh_idx);
        if (mesh_intersect(mesh, ray, isect)) {
            do_intersect = true;
        }
    }
    return do_intersect;
}

bool scene_do_intersect(const struct Scene *scene, const Ray *ray) {
    size_t mesh_num = scene_size(scene);
    bool do_intersect = false;
    for (size_t mesh_idx = 0; mesh_idx < mesh_num; ++mesh_idx) {
        struct Mesh *mesh = scene_get(scene, mesh_idx);
        if (mesh_do_intersect(mesh, ray)) {
            do_intersect = true;
            break;
        }
    }
    return do_intersect;
}