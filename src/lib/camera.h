#ifndef TEAM32_CAMERA_H
#define TEAM32_CAMERA_H

#include "vector.h"
#include "ray.h"

struct Camera {
    Vector c;  // pinhole
    // camera frame basis, with ex pointing image's right, ey pointing image's down, and ez pointing view direction
    Vector ex, ey, ez;
    size_t W, H;  // pixel resolution in horizontal/vertical axis
    float fov;  // field of view in angles
    float aspect;  // W / H
};

// Set camera basis vectors by specifying eye, target, and up vectors
void cam_look_at(struct Camera *camera, const Vector *eye, const Vector *target, const Vector *up);

void cam_set_resolution(struct Camera *camera, size_t W, size_t H);

// Given image pixel coordinate 0 <= px < W, 0 <= py < H, return the ray pointing from pinhole to the image pixel
// in world coordinates. `sample` is used to offsetting the pixel a bit for anti-aliasing
Ray generate_ray(const struct Camera * camera, size_t px, size_t py, Vector2f sample);

#endif //TEAM32_CAMERA_H
