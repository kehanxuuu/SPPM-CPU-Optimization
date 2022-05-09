#ifndef _ARRAY_POINTER_H
#define _ARRAY_POINTER_H

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

#endif