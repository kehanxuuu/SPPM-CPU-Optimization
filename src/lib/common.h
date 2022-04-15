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
struct Geometry;
struct Intersection;
struct Scene;
struct Camera;
struct RayTracing;
struct SPPM;

#define NOT_USED(x) ((void)(x))

// a few useful constants
#undef M_PI
#define M_PI         3.14159265358979323846f
#define INV_PI       0.31830988618379067154f

#endif //TEAM32_COMMON_H
