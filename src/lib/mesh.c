#include "mesh.h"

void sphere_init(struct Sphere *sphere, Vector c, float r) {
    sphere->c = c;
    sphere->r = r;
    sphere->r2 = r*r;
}

Ray sphere_surface_sample(struct Sphere *sphere, Vector2f sample1, Vector2f sample2){
    float z = sample1.x * 2 - 1;
    float theta = sample1.y * 2 * M_PI - M_PI;
    float x = sinf(theta) * sqrtf(sphere->r2 - z * z);
    float y = cosf(theta) * sqrtf(sphere->r2 - z * z);
    Vector normal = (Vector){x, y, z};
    Vector origin = vv_add(&sphere->c, &normal);

    v_normalize(&normal);
    Vector direction = square_to_cosine_hemisphere(&sample2, &normal);
    struct Ray ray;
    ray_init(&ray, &origin, &direction, INFINITY);
    return ray;
}

void geometry_init_sphere(struct Geometry *geometry, struct Sphere *sphere) {
    geometry->type = SPHERE;
    geometry->data = sphere;
}

void mesh_init(struct Mesh *mesh, struct Geometry *geometry, enum Material material, Vector albedo, Vector emission) {
    mesh->geometry = geometry;
    // randomly initialize
    mesh->material = material;
    mesh->albedo = albedo;
    mesh->emission = emission;
}

void mesh_init_sphere(struct Mesh *mesh, Vector c, float r, enum Material material, Vector albedo, Vector emission) {
    struct Sphere *sphere = (struct Sphere *)malloc(sizeof(struct Sphere));
    sphere_init(sphere, c, r);
    struct Geometry *geometry = (struct Geometry *)malloc(sizeof(struct Geometry));
    geometry_init_sphere(geometry, sphere);
    mesh_init(mesh, geometry, material, albedo, emission);
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
                isect->wi = v_normalized(&ray->d);
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