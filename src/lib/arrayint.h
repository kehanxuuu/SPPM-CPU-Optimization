#ifndef _ARRAY_INT_H
#define _ARRAY_INT_H

typedef struct {
    size_t size;
    size_t capacity;
    int *data;
} IntArray;

void arr_add_int(IntArray *arr, void *item);

int arr_get_int(const IntArray *arr, size_t index);

void arr_set_int(IntArray *arr, size_t index, int value);

void arr_set_add_int(IntArray *arr, size_t index, int value);

size_t arr_size_int(const IntArray *arr);

void arr_init_int(IntArray *arr, size_t capacity, size_t initial_size);

void arr_free_int(IntArray *arr);

#endif