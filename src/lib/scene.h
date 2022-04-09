#ifndef TEAM32_SCENE_H
#define TEAM32_SCENE_H

#include "vector.h"
#include "intersection.h"
#include "array.h"

struct Scene {
    Array meshes;
};

// Add a mesh to `scene->meshes`
void scene_add(struct Scene *scene, struct Mesh *mesh);

// Get a mesh pointer indicated by index
struct Mesh *scene_get(struct Scene *scene, size_t index);

// Returns number of meshes
size_t scene_size(const struct Scene *scene);

// Find the intersection between the scene meshes and the ray, stores the time to `ray.t_max`, and the intersection data to `isect`
// Returns if intersect
// Please switch case every possible geometry type and implement the intersection code accordingly
bool scene_intersect(struct Scene *scene, Ray *ray, struct Intersection *isect);

// Do only predicate: shadow ray test
bool scene_do_intersect(const struct Mesh *mesh, const Ray *ray);

#endif //TEAM32_SCENE_H
