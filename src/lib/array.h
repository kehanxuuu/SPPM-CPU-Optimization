#ifndef _ARRAY_H
#define _ARRAY_H

#include <stddef.h>

#ifndef _ARRAY_INCRE_MULT
    #define _ARRAY_INCRE_MULT 2
#endif

// general array type
typedef struct {
    size_t size;
    size_t capacity;
    size_t item_size;
    void *data;
} Array;

void arr_add(Array *arr, void *item);

void *arr_get(const Array *arr, size_t index);

size_t arr_size(const Array *arr);

void arr_init(Array *arr, size_t capacity, size_t initial_size, size_t item_size);

void arr_free(Array *arr);

// type-specific array
// float (currently not used, maybe of use in the future)
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

// pointer
typedef struct {
    size_t size;
    size_t capacity;
    char **data;
} PointerArray;

void arr_add_pointer(PointerArray *arr, void *item);

char *arr_get_pointer(const PointerArray *arr, size_t index);

size_t arr_size_pointer(const PointerArray *arr);

void arr_init_pointer(PointerArray *arr, size_t capacity, size_t initial_size);

void arr_free_pointer(PointerArray *arr);

//

#endif