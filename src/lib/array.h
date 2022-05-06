#ifndef _ARRAY_H
#define _ARRAY_H

#include <stddef.h>

#ifndef _ARRAY_INCRE_MULT
    #define _ARRAY_INCRE_MULT 2
#endif

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

#endif