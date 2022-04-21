#include "arrayfixed2d.h"
#include <stddef.h>
void* arrfixed2d_get(ArrayFixed2D *arr, size_t x, size_t y){
    return arr_get(&arr->arr, arrfixed2d_translate(arr, x, y));
}

void arrfixed2d_set(ArrayFixed2D *arr, size_t x, size_t y, void *item){
    Array* internal_arr = &arr->arr;
    void* target_addr = (uint8_t *) internal_arr->data + arrfixed2d_translate(arr, x, y) * internal_arr->item_size;
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

size_t arrfixed2d_translate(ArrayFixed2D *arr, size_t x, size_t y){
    return x * arr->height + y;
}

