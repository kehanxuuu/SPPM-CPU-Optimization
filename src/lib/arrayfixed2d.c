#include "arrayfixed2d.h"
#include <assert.h>
#include <stddef.h>

void* arrfixed2d_get(ArrayFixed2D *arr, size_t y, size_t x){
    assert(x < arr->width && y < arr->height);
    return arr_get(&arr->arr, arrfixed2d_translate(arr, y, x));
}

void arrfixed2d_set(ArrayFixed2D *arr, size_t y, size_t x, void *item){
    assert(x < arr->width && y < arr->height);
    Array* internal_arr = &arr->arr;
    void* target_addr = (uint8_t *) internal_arr->data + arrfixed2d_translate(arr, y, x) * internal_arr->item_size;
    memcpy(target_addr, item, arr->arr.item_size);
}

void arrfixed2d_init(ArrayFixed2D *arr, size_t height, size_t width, size_t item_size){
    arr_init(&arr->arr, height * width, height * width, item_size);
    arr->width = width;
    arr->height = height;
}

void arrfixed2d_free(ArrayFixed2D *arr){
    arr_free(&arr->arr);
}

size_t arrfixed2d_translate(ArrayFixed2D *arr, size_t y, size_t x){
    return y * arr->width + x;
}

