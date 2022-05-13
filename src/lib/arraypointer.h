#ifndef _ARRAY_POINTER_H
#define _ARRAY_POINTER_H

#include "array.h"

typedef struct {
    size_t size;
    size_t capacity;
    char **data;
} PointerArray;

static inline void arr_add_pointer(PointerArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(char *));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(char *);
    memcpy(target_addr, item, sizeof(char *));
    arr->size++;
}

static inline char *arr_get_pointer(const PointerArray *arr, size_t index) {
    assert(index < arr->size);
    return arr->data[index];
}

static inline void arr_set_pointer(PointerArray *arr, size_t index, char * value) {
    assert(index < arr->size);
    arr->data[index] = value;
}

static inline size_t arr_size_pointer(const PointerArray *arr) {
    return arr->size;
}

static inline void arr_init_pointer(PointerArray *arr, size_t capacity, size_t initial_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->data = malloc(capacity * sizeof(char *));
}

static inline void arr_free_pointer(PointerArray *arr) {
    free(arr->data);
}

#endif