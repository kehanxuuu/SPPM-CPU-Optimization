#ifndef TEAM32_ARRAYFIXED2D_H
#define TEAM32_ARRAYFIXED2D_H

#include "array.h"
#include "arrayfixed2d.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

typedef struct{
    Array arr;
    size_t width;
    size_t height;
} ArrayFixed2D;

static inline size_t arrfixed2d_translate(ArrayFixed2D *arr, size_t y, size_t x) {
    return y * arr->width + x;
}

static inline void* arrfixed2d_get(ArrayFixed2D *arr, size_t y, size_t x) {
    assert(x < arr->width && y < arr->height);
    return arr_get(&arr->arr, arrfixed2d_translate(arr, y, x));
}

static inline void *arrfixed2d_get_1D(ArrayFixed2D *arr, size_t index) {
    assert(index < arr->width * arr->height);
    return arr_get(&arr->arr, index);
}

static inline void arrfixed2d_set(ArrayFixed2D *arr, size_t y, size_t x, void *item) {
    assert(x < arr->width && y < arr->height);
    Array *internal_arr = &arr->arr;
    void *target_addr = (uint8_t *) internal_arr->data + arrfixed2d_translate(arr, y, x) * internal_arr->item_size;
    memcpy(target_addr, item, arr->arr.item_size);
}

static inline void arrfixed2d_init(ArrayFixed2D *arr, size_t height, size_t width, size_t item_size) {
    arr_init(&arr->arr, height * width, height * width, item_size);
    arr->width = width;
    arr->height = height;
}

static inline void arrfixed2d_free(ArrayFixed2D *arr) {
    arr_free(&arr->arr);
}

#endif //TEAM32_ARRAYFIXED2D_H
