#include "array.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void arr_add(Array* arr, void* item) {
    if (arr->size == arr->capacity) {
        arr->capacity = _ARRAY_INCRE_MULT * arr->capacity;
        arr->data = realloc(arr->data, arr->capacity * arr->item_size);
    }

    void* target_addr = (uint8_t*)arr->data + arr->size * arr->item_size;
    memcpy(target_addr, item, arr->item_size);
    arr->size++;
}

void* arr_get(Array* arr, size_t index) {
    return (uint8_t*)arr->data + index * arr->item_size;
}

size_t arr_size(Array* arr) {
    return arr->size;
}

void arr_init(Array* arr, size_t initial_size, size_t item_size) {
    arr->size = 0;
    arr->capacity = initial_size;
    arr->item_size = item_size;
    arr->data = malloc(initial_size * item_size);
}

void arr_free(Array* arr) {
    free(arr->data);
}