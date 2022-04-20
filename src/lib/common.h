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
#include "array.h"

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
struct RayTracing;
struct SPPM;

#define NOT_USED(x) ((void)(x))

// a few useful constants
#undef M_PI
#define M_PI         3.14159265358979323846f
#define INV_PI       0.31830988618379067154f

#endif //TEAM32_COMMON_H
