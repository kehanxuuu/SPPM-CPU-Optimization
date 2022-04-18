#include "mesh.h"

struct Sphere *sphere_init(Vector c, float r) {
    struct Sphere *sphere = (struct Sphere *)malloc(sizeof(struct Sphere));
    sphere->c = c;
    sphere->r = r;
    sphere->r2 = r*r;
    return sphere;
}

struct Geometry *geometry_init_sphere(struct Sphere *sphere) {
    struct Geometry *geometry = (struct Geometry *)malloc(sizeof(struct Geometry));
    geometry->type = SPHERE;
    geometry->data = sphere;
    return geometry;
}

struct Mesh *mesh_init(struct Geometry *geometry) {
    struct Mesh *mesh = (struct Mesh *)malloc(sizeof(struct Mesh));
    mesh->geometry = geometry;
    // randomly initialize
    mesh->material = DIFFUSE;
    mesh->albedo.x = mesh->albedo.y = mesh->albedo.z = 0.5;
    mesh->emission.x = mesh->emission.y = mesh->emission.z = 0;
    return mesh;
}

struct Mesh *mesh_init_sphere(Vector c, float r) {
    struct Sphere *sphere = sphere_init(c, r);
    struct Geometry *geometry = geometry_init_sphere(sphere);
    struct Mesh *mesh = mesh_init(geometry);
    return mesh;
}

void geometry_free(struct Geometry *geometry) {
    free(geometry->data);
    free(geometry);
}

void mesh_free(struct Mesh *mesh) {
    geometry_free(mesh->geometry);
    free(mesh);
}

bool mesh_intersect(struct Mesh *mesh, Ray *ray, struct Intersection *isect) {
    struct Geometry *geometry = mesh->geometry;
    // determine intersect point according to geometry
    switch(geometry->type) {
        case SPHERE: {
            // t^2 d*d + 2t d*(O-C) + (O-C)*(O-C) - r*r = 0
            struct Sphere *sphere = (struct Sphere *)geometry->data;
            Vector oc = vv_sub(&ray->o, &sphere->c);
            float a = vv_dot(&ray->d, &ray->d);
            float half_b = vv_dot(&ray->d, &oc);
            float c = vv_dot(&oc, &oc) - sphere->r2;
            float discriminant = half_b * half_b - a * c;
            if (discriminant < 0) {
                return false;
            }
            else {
                float sqrt_d = sqrtf(discriminant);
                float root = (-half_b - sqrt_d) / a;
                if (root < 0 || root > ray->t_max) {
                    root = (-half_b + sqrt_d) / a;
                    if (root < 0 || root > ray->t_max) {
                        return false;
                    }
                }
                ray->t_max = root;
                isect->hit = mesh;
                isect->p = ray_at(ray, root);
                Vector sphere_normal = vv_sub(&isect->p, &sphere->c);
                isect->n = v_normalized(&sphere_normal);
                isect->wi = ray->d;
            }
            break;
        }

        default:
            break;
    }

    return true;
}


bool mesh_do_intersect(const struct Mesh *mesh, Ray *ray) {
    struct Geometry *geometry = mesh->geometry;
    switch(geometry->type) {
        case SPHERE: {
            // t^2 d*d + 2t d*(O-C) + (O-C)*(O-C) - r*r = 0
            struct Sphere *sphere = (struct Sphere *)geometry->data;
            Vector oc = vv_sub(&ray->o, &sphere->c);
            float a = vv_dot(&ray->d, &ray->d);
            float half_b = vv_dot(&ray->d, &oc);
            float c = vv_dot(&oc, &oc) - sphere->r2;
            float discriminant = half_b * half_b - a * c;
            if (discriminant < 0) {
                return false;
            }
            else {
                float sqrt_d = sqrtf(discriminant);
                float root = (-half_b - sqrt_d) / a;
                if (root < 0 || root > ray->t_max) {
                    root = (-half_b + sqrt_d) / a;
                    if (root < 0 || root > ray->t_max) {
                        return false;
                    }
                }
                ray->t_max = root;
            }
            break;
        }

        default:
            break;
    }

    return true;
}