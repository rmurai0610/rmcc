/* Vector of void pointers */
#include "self-c.h"

Vector *vector_init() {
    Vector *vec = (Vector *)malloc(sizeof(Vector));
    vec->size = 1;
    vec->count = 0;
    vec->data = (void **)malloc(sizeof(void *));
    return vec;
}

void *vector_get(Vector *vec, int index) {
    if (index > vec->count) {
        error("%s: index (%d) out of range (%d)", __func__, index, vec->count);
    }
    return vec->data[index];
}

void vector_add(Vector *vec, void *data) {
    if (vec->size == vec->count) {
        /* Resize vector */
        vector_resize(vec, vec->size * 2);
    }
    vec->data[vec->count++] = data;
}

void vector_delete(Vector *vec, int index) {
    if (index > vec->count) {
        error("%s: index (%d) out of range (%d)", __func__, index, vec->count);
    }
    vec->data[index] = NULL;
    vec->count--;
    size_t n = vec->count - index;
    if (n == 0) {
        return;
    }
    memmove(&vec->data[index], &vec->data[index + 1], sizeof(void *) * n);
    vec->data[vec->count] = NULL;
}

void vector_resize(Vector *vec, int size) {
    vec->size = size;
    vec->data = (void **)realloc(vec->data, sizeof(void *) * size);
}
