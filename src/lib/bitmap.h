#ifndef TEAM32_BITMAP_H
#define TEAM32_BITMAP_H

#include "vector.h"

// Struct for storing image
struct Bitmap {
    size_t W, H;  // resolution
    Array pixels;  // row major
};

#ifdef __cplusplus

extern "C" void bitmap_save_exr(struct Bitmap *bitmap, char *filename);

#else  // on C compiler

void bitmap_init(struct Bitmap *bitmap, size_t W, size_t H);

void bitmap_set(struct Bitmap *bitmap, size_t px, size_t py, const Vector *color);

Vector *bitmap_get(struct Bitmap *bitmap, size_t px, size_t py);

void bitmap_free(struct Bitmap *bitmap);

void bitmap_save_exr(struct Bitmap *bitmap, char *filename);

double bitmap_checksum(struct Bitmap *bitmap);

#endif

#endif //TEAM32_BITMAP_H
