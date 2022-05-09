#ifndef _ARRAY_FLOAT_H
#define _ARRAY_FLOAT_H

typedef struct {
    size_t size;
    size_t capacity;
    float *data;
} FloatArray;

void arr_add_float(FloatArray *arr, void *item);

float arr_get_float(const FloatArray *arr, size_t index);

size_t arr_size_float(const FloatArray *arr);

void arr_init_float(FloatArray *arr, size_t capacity, size_t initial_size);

void arr_free_float(FloatArray *arr);

#endif