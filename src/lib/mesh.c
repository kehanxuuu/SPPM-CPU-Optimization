#include "mesh.h"
#include "warping.h"

void sphere_init(struct Sphere *sphere, Vector c, float r) {
    sphere->c = c;
    sphere->r = r;
    sphere->r2 = r * r;
}

Ray sphere_surface_photon_sample(struct Sphere *sphere, Vector2f sample1, Vector2f sample2, float *pdf_pos, float *pdf_dir) {
    Vector normal = square_to_uniform_sphere(sample1);
    Vector origin = vvs_fma(&sphere->c, &normal, sphere->r + EPSILON);
    *pdf_pos = 1.0f / (2 * M_PI * sphere->r2);
    Vector direction = square_to_uniform_hemisphere(sample2, &normal);
    *pdf_dir = M_1_PI * 0.5;
//    Vector direction = square_to_cosine_hemisphere(sample2, &normal);
//    *pdf_dir = vv_dot(&normal, &direction) * INV_PI;  // cos_theta / Pi
    return (Ray) {origin, direction, INFINITY};
}

void geometry_init_sphere(struct Geometry *geometry, struct Sphere *sphere) {
    geometry->type = SPHERE;
    geometry->data = sphere;
}

void mesh_init(struct Mesh *mesh, struct Geometry *geometry, enum Material material, Vector albedo, Vector emission, float ir) {
    mesh->geometry = geometry;
    // randomly initialize
    mesh->material = material;
    mesh->albedo = albedo;
    mesh->emission = emission;
    mesh->ir = ir;
}

Mesh *mesh_make_sphere(Vector c, float r, enum Material material, Vector albedo, Vector emission, float ir) {
    Mesh *mesh = (Mesh *) malloc(sizeof(Mesh));
    struct Sphere *sphere = (struct Sphere *) malloc(sizeof(struct Sphere));
    sphere_init(sphere, c, r);
    struct Geometry *geometry = (struct Geometry *) malloc(sizeof(struct Geometry));
    geometry_init_sphere(geometry, sphere);
    mesh_init(mesh, geometry, material, albedo, emission, ir);
    return mesh;
}

void geometry_free(struct Geometry *geometry) {
    free(geometry->data);
}

void mesh_free(struct Mesh *mesh) {
    geometry_free(mesh->geometry);
    free(mesh->geometry);
}

bool mesh_intersect(struct Mesh *mesh, struct Ray *ray, struct Intersection *isect) {
    struct Geometry *geometry = mesh->geometry;
    // determine intersect point according to geometry
    switch (geometry->type) {
        case SPHERE: {
            // t^2 d*d + 2t d*(O-C) + (O-C)*(O-C) - r*r = 0
            struct Sphere *sphere = (struct Sphere *) geometry->data;
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
                if (root > ray->t_max) return false;
                if (root < 0) {
                    root = (-half_b + sqrt_d) / a;
                    if (root < 0 || root > ray->t_max) {
                        return false;
                    }
                    // Intersect at interior
                    isect->interior = true;
                }
                else {
                    // Intersect at exterior
                    isect->interior = false;
                }
                ray->t_max = root;
                isect->hit = mesh;
                isect->p = ray_at(ray, root);
                isect->n = vv_sub(&isect->p, &sphere->c);
                v_normalize(&isect->n);
                if (isect->interior) vs_muleq(&isect->n, -1);
                isect->wi = v_normalized(&ray->d);
            }
            break;
        }

        default:
        UNIMPLEMENTED;
    }

    return true;
}


bool mesh_do_intersect(const struct Mesh *mesh, const Ray *ray) {
    struct Geometry *geometry = mesh->geometry;
    switch (geometry->type) {
        case SPHERE: {
            // t^2 d*d + 2t d*(O-C) + (O-C)*(O-C) - r*r = 0
            struct Sphere *sphere = (struct Sphere *) geometry->data;
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
                if (root > ray->t_max) return false;
                if (root < 0) {
                    root = (-half_b + sqrt_d) / a;
                    if (root < 0 || root > ray->t_max) {
                        return false;
                    }
                }
            }
            break;
        }

        default:
        UNIMPLEMENTED;
    }

    return true;
}