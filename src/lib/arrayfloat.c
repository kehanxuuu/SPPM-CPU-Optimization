#include "array.h"
#include "arrayfloat.h"

void arr_add_float(FloatArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(float));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(float);
    memcpy(target_addr, item, sizeof(float));
    arr->size++;
}

inline float arr_get_float(const FloatArray *arr, size_t index) {
    assert(index < arr->size);
    return (float) arr->data[index];
}

inline void arr_set_float(FloatArray *arr, size_t index, float value) {
    assert(index < arr->size);
    arr->data[index] = value;
}

inline void arr_set_add_float(FloatArray *arr, size_t index, float value) {
    assert(index < arr->size);
    arr->data[index] += value;
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