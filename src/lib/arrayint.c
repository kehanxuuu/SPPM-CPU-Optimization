#include "array.h"
#include "arrayint.h"

void arr_add_int(IntArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(int));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(int);
    memcpy(target_addr, item, sizeof(int));
    arr->size++;
}

inline int arr_get_int(const IntArray *arr, size_t index) {
    assert(index < arr->size);
    return (int) arr->data[index];
}

inline void arr_set_int(IntArray *arr, size_t index, int value) {
    assert(index < arr->size);
    arr->data[index] = value;
}

inline void arr_set_add_int(IntArray *arr, size_t index, int value) {
    assert(index < arr->size);
    arr->data[index] += value;
}

size_t arr_size_int(const IntArray *arr) {
    return arr->size;
}

void arr_init_int(IntArray *arr, size_t capacity, size_t initial_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->data = malloc(capacity * sizeof(int));
}

void arr_free_int(IntArray *arr) {
    free(arr->data);
}