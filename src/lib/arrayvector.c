#include "array.h"
#include "arrayvector.h"

void arr_add_vector(VectorArray *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * sizeof(Vector));
    }

    void *target_addr = (uint8_t *) arr->data + arr->size * sizeof(Vector);
    memcpy(target_addr, item, sizeof(Vector));
    arr->size++;
}

Vector arr_get_vector(const VectorArray *arr, size_t index) {
    assert(index < arr->size);
    return arr->data[index];
}

size_t arr_size_vector(const VectorArray *arr) {
    return arr->size;
}

void arr_init_vector(VectorArray *arr, size_t capacity, size_t initial_size) {
    assert(initial_size <= capacity);
    arr->size = initial_size;
    arr->capacity = capacity;
    arr->data = malloc(capacity * sizeof(Vector));
}

void arr_free_vector(VectorArray *arr) {
    free(arr->data);
}