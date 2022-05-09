#ifndef _ARRAY_VECTOR_H
#define _ARRAY_VECTOR_H

#include "vector.h"

typedef struct {
    size_t size;
    size_t capacity;
    Vector *data;
} VectorArray;

void arr_add_vector(VectorArray *arr, void *item);

Vector arr_get_vector(const VectorArray *arr, size_t index);

size_t arr_size_vector(const VectorArray *arr);

void arr_init_vector(VectorArray *arr, size_t capacity, size_t initial_size);

void arr_free_vector(VectorArray *arr);

#endif