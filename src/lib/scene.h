#ifndef TEAM32_SCENE_H
#define TEAM32_SCENE_H

#include "vector.h"
#include "array.h"
#include "mesh.h"
#include "intersection.h"

struct Scene {
    Array meshes; // store pointers to meshes
    size_t n_meshes;
    size_t n_emitters;
    Mesh **emitters;
    float *accum_probabilities;  // accumulated probabilities for emitter sample
};

// Initialize scene struct
void scene_init(struct Scene *scene);

// Add a mesh to `scene->meshes`
void scene_add(struct Scene *scene, struct Mesh *mesh);

// Get a mesh pointer indicated by index
struct Mesh *scene_get(const struct Scene *scene, size_t index);

// Free the scene struct
void scene_free(struct Scene *scene);

// Find the intersection between the scene meshes and the ray, stores the time to `ray.t_max`, and the intersection data to `isect`
// Returns if intersect
// Please switch case every possible geometry type and implement the intersection code accordingly
bool scene_intersect(const struct Scene *scene, Ray *ray, struct Intersection *isect);

// Do only predicate: shadow ray test
bool scene_do_intersect(const struct Scene *scene, const Ray *ray);

// Must be called when finishing adding all meshes
void scene_finish(struct Scene *scene);

// Sample an emitter based on surface area
struct Mesh *sample_emitter(const struct Scene *scene, float sample, float *pdf);

// Estimate direct lighting from given intersection
Vector estimate_direct_lighting(const struct Scene *scene, struct Intersection *isect);

#endif //TEAM32_SCENE_H
