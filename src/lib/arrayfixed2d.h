#ifndef TEAM32_ARRAYFIXED2D_H
#define TEAM32_ARRAYFIXED2D_H

#include "array.h"
#include <stdint.h>
#include <string.h>

typedef struct{
    Array arr;
    size_t width;
    size_t height;
} ArrayFixed2D;

void* arrfixed2d_get(ArrayFixed2D *arr, size_t x, size_t y);

void arrfixed2d_set(ArrayFixed2D *arr, size_t x, size_t y, void *item);

void arrfixed2d_init(ArrayFixed2D *arr, size_t height, size_t width, size_t item_size);

void arrfixed2d_free(ArrayFixed2D *arr);

size_t arrfixed2d_translate(ArrayFixed2D *arr, size_t x, size_t y);

#endif //TEAM32_ARRAYFIXED2D_H
