#include "mesh.h"

bool mesh_intersect(struct Mesh *mesh, Ray *ray, struct Intersection *isect) {
    struct Geometry geometry = mesh->geometry;
    // determine intersect point according to geometry
    switch(geometry.type) {
        case Sphere: {
            // t^2 d*d + 2t d*(O-C) + (O-C)*(O-C) - r*r = 0
            struct Sphere *sphere = (struct Sphere *)geometry.data;
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
                if (root < 0) {
                    root = (-half_b + sqrt_d) / a;
                    if (root < 0) {
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
    struct Geometry geometry = mesh->geometry;
    switch(geometry.type) {
        case Sphere: {
            // t^2 d*d + 2t d*(O-C) + (O-C)*(O-C) - r*r = 0
            struct Sphere *sphere = (struct Sphere *)geometry.data;
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
                if (root < 0) {
                    root = (-half_b + sqrt_d) / a;
                    if (root < 0) {
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