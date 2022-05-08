#ifndef TEAM32_MESH_H
#define TEAM32_MESH_H

#include "vector.h"
#include "intersection.h"
#include "ray.h"

// Generic geometry class
struct Geometry {
    enum Type {
        SPHERE = 0,
        // Plane
        // Triangle
        // ...
    } type;
    void *data;  // points to e.g. a Sphere or Triangle struct
};

// An instance in the scene, carrying geometry and material information
struct Mesh {
    struct Geometry *geometry;

    enum Material {
        DIFFUSE = 0,
        SPECULAR,
        DIELECTRIC,
    } material;

    // Color
    Vector albedo;
    Vector emission;

    // Index of Reflection
    float ir;

    // Texture?
};

struct Sphere {
    Vector c;
    float r, r2;  // radius and radius^2
};

// Temporary initialization and free methods
void sphere_init(struct Sphere *sphere, Vector c, float r);

Ray sphere_surface_photon_sample(struct Sphere *sphere, Vector2f sample1, Vector2f sample2, float *pdf_pos, float *pdf_dir);

void geometry_init_sphere(struct Geometry *geometry, struct Sphere *sphere);

void mesh_init(struct Mesh *mesh, struct Geometry *geometry, enum Material material, Vector albedo, Vector emission, float ir);

Mesh *mesh_make_sphere(Vector c, float r, enum Material material, Vector albedo, Vector emission, float ir);

void geometry_free(struct Geometry *geometry);

void mesh_free(struct Mesh *mesh);

// Find the intersection between the mesh and the ray, stores the time to `ray.t_max`, and the intersection data to `isect`
// Returns if intersect
// Please switch case every possible geometry type and implement the intersection code accordingly
bool mesh_intersect(struct Mesh *mesh, struct Ray *ray, struct Intersection *isect);

// Do only predicate: shadow ray test
// store intersect time in ray to facilitate scene intersection
bool mesh_do_intersect(const struct Mesh *mesh, const Ray *ray);

#endif //TEAM32_MESH_H
