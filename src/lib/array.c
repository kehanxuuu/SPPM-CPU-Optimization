#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "array.h"

// general type
void arr_add(Array *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * arr->item_size);
    }
    
    void *target_addr = (uint8_t *) arr->data + arr->size * arr->item_size;
    memcpy(target_addr, item, arr->item_size);
    arr->size++;
}

void *arr_get(const Array *arr, size_t index) {
    assert(index < arr->size);
    return (uint8_t *) arr->data + index * arr->item_size;
}

size_t arr_size(const Array *arr) {
    return arr->size;
}

void arr_init(Array *arr, size_t capacity, size_t initial_size, size_t item_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->item_size = item_size;
    arr->data = malloc(capacity * item_size);
}

void arr_free(Array *arr) {
    free(arr->data);
}


// float type
void arr_add_float(FloatArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(float));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(float);
    memcpy(target_addr, item, sizeof(float));
    arr->size++;
}

float arr_get_float(const FloatArray *arr, size_t index) {
    assert(index < arr->size);
    return (float) arr->data[index];
}

size_t arr_size_float(const FloatArray *arr) {
    return arr->size;
}

void arr_init_float(FloatArray *arr, size_t capacity, size_t initial_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->data = malloc(capacity * sizeof(float));
}

void arr_free_float(FloatArray *arr) {
    free(arr->data);
}


// pointer type
void arr_add_pointer(PointerArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(char *));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(char *);
    memcpy(target_addr, item, sizeof(char *));
    arr->size++;
}

char *arr_get_pointer(const PointerArray *arr, size_t index) {
    assert(index < arr->size);
    return arr->data[index];
}

size_t arr_size_pointer(const PointerArray *arr) {
    return arr->size;
}

void arr_init_pointer(PointerArray *arr, size_t capacity, size_t initial_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->data = malloc(capacity * sizeof(char *));
}

void arr_free_pointer(PointerArray *arr) {
    free(arr->data);
}