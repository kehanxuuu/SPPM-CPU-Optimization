#ifndef _ARRAY_VECTOR_H
#define _ARRAY_VECTOR_H

#include "vector.h"
#include "array.h"

typedef struct {
    size_t size;
    size_t capacity;
    Vector *data;
} VectorArray;

static inline void arr_add_vector(VectorArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(Vector));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(Vector);
    memcpy(target_addr, item, sizeof(Vector));
    arr->size++;
}

static inline Vector arr_get_vector(const VectorArray *arr, size_t index) {
    assert(index < arr->size);
    return arr->data[index];
}

static inline void arr_set_vector(VectorArray *arr, size_t index, Vector value) {
    assert(index < arr->size);
    arr->data[index] = value;
}

static inline void arr_set_add_vector(VectorArray *arr, size_t index, Vector value) {
    assert(index < arr->size);
    vv_addeq(&arr->data[index], &value);
}

static inline size_t arr_size_vector(const VectorArray *arr) {
    return arr->size;
}

static inline void arr_init_vector(VectorArray *arr, size_t capacity, size_t initial_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->data = malloc(capacity * sizeof(Vector));
}

static inline void arr_free_vector(VectorArray *arr) {
    free(arr->data);
}

#endif