#include "bitmap.h"

void bitmap_init(struct Bitmap *bitmap, size_t W, size_t H) {
    bitmap->W = W;
    bitmap->H = H;
    arr_init(&bitmap->pixels, W * H, W * H, sizeof(Vector));
}

void bitmap_set(struct Bitmap *bitmap, size_t px, size_t py, const Vector *color) {
    size_t index = px + py * bitmap->W;
    *(Vector *) arr_get(&bitmap->pixels, index) = *color;
}

Vector *bitmap_get(struct Bitmap *bitmap, size_t px, size_t py) {
    size_t index = px + py * bitmap->W;
    return (Vector *) arr_get(&bitmap->pixels, index);
}

double bitmap_checksum(struct Bitmap *bitmap) {
    double sum = 0;
    for(int i = 0; i < bitmap->H; i++){
        for(int j = 0; j < bitmap->W; j++){
            size_t index = j + i * bitmap->W;
            Vector* cur_vector = arr_get(&bitmap->pixels, index);
            sum += v_norm(cur_vector);
        }
    }
    return sum;
}

void bitmap_free(struct Bitmap *bitmap) {
    arr_free(&bitmap->pixels);
}
