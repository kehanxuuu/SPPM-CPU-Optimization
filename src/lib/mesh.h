#ifndef TEAM32_MESH_H
#define TEAM32_MESH_H

#include "vector.h"

// Generic geometry class
struct Geometry {
    enum Type {
        Sphere,
        // Plane
        // Triangle
        // ...
    } type;
    void *data;  // points to e.g. a Sphere or Triangle struct
};

// An instance in the scene, carrying geometry and material information
struct Mesh {
    struct Geometry geometry;

    enum Material {
        Diffuse = 0,
        Specular,
        Dielectric,
    } material;

    // Color
    Vector albedo;
    Vector emission;

    // Texture?
};

struct Sphere {
    Vector c;
    float r, r2;  // radius and radius^2
};

// Find the intersection between the mesh and the ray, stores the time to `ray.t_max`, and the intersection data to `isect`
// Returns if intersect
// Please switch case every possible geometry type and implement the intersection code accordingly
bool mesh_intersect(struct Mesh *mesh, Ray *ray, struct Intersection *isect);

// Do only predicate: shadow ray test
bool mesh_do_intersect(const struct Mesh *mesh, const Ray *ray);


#endif //TEAM32_MESH_H
