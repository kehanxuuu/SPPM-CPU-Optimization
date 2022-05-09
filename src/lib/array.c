#include "array.h"

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