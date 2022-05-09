#ifndef TEAM32_COMMON_H
#define TEAM32_COMMON_H

// Includes:
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <float.h>

// Forwarding definitions:
struct Mesh;
typedef struct Mesh Mesh;
struct Geometry;
typedef struct Geometry Geometry;
struct Intersection;
typedef struct Intersection Intersection;
struct Scene;
typedef struct Scene Scene;
struct Camera;
typedef struct Camera Camera;
struct Ray;
typedef struct Ray Ray;
struct Bitmap;
typedef struct Bitmap Bitmap;
struct Sphere;
typedef struct Sphere Sphere;

#define NOT_USED(x) ((void)(x))
#define UNIMPLEMENTED fprintf(stderr, "Unimplemented error at %s:%d\n", __FILE__, __LINE__); exit(1)
#define UNREACHABLE fprintf(stderr, "Unreachable code at %s:%d\n", __FILE__, __LINE__); exit(1)

// a few useful constants
#undef M_PI
#define M_PI         3.14159265358979323846f
#define INV_PI       0.31830988618379067154f
#define EPSILON      1e-2f

#endif //TEAM32_COMMON_H
