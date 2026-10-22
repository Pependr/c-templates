#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdlib.h>
#include <stdbool.h>

#define VECTOR_CONCAT1(a, b) a##b
#define VECTOR_CONCAT2(a, b) VECTOR_CONCAT1(a, b)

#define VECTOR_NAME VECTOR_CONCAT2(vector_, SUFFIX)
#define VECTOR_TYPE VECTOR_CONCAT2(VECTOR_NAME, _s)
#define VECTOR_METHOD(name) VECTOR_CONCAT2(VECTOR_NAME, _##name)

#endif // VECTOR_H_


#ifndef T
#   error "T must be defined before including vector.h"
#endif

#ifndef SUFFIX
#   define SUFFIX T
#endif

#if defined(DEFAULT_CAP) && DEFAULT_CAP <= 0
#   error "DEFAULT_CAP must be a positive integer"
#endif


typedef struct {
    T *items;
    size_t len, cap;
    bool is_static;
} VECTOR_TYPE;


static inline bool VECTOR_METHOD(init)(VECTOR_TYPE *vec) {
    if (vec->items != NULL && vec->cap == 0)
        return false;

    if (vec->cap == 0)
#ifdef DEFAULT_CAP
        vec->cap = DEFAULT_CAP;
#else
        return false;
#endif

    if (vec->items == NULL) {
        vec->items = malloc(vec->cap * sizeof(T));
        if (!vec->items) return false;
        vec->len = 0;
    }

    return true;
}


static inline bool VECTOR_METHOD(append)(VECTOR_TYPE *vec, T item) {
    if (vec->len >= vec->cap) {
        if (vec->is_static) return false;

        vec->cap *= 2;
        vec->items = realloc(vec->items, vec->cap * sizeof(T));
        if (!vec->items) return false;
    }

    vec->items[vec->len++] = item;

    return true;
}


static inline bool VECTOR_METHOD(pop)(VECTOR_TYPE *vec, T *dest) {
    if (vec->len == 0) return false;

    if (dest)
        *dest = vec->items[--(vec->len)];
    else
        vec->len -= 1;

    if (vec->len != 0 && !vec->is_static && vec->cap / vec->len >= 2) {
        vec->cap /= 2;
        vec->items = realloc(vec->items, vec->cap * sizeof(T));
        if (!vec->items) return false;
    }

    return true;
}


static inline bool VECTOR_METHOD(swap_remove)(VECTOR_TYPE *vec, size_t idx) {
    if (idx >= vec->len) return false;
    return VECTOR_METHOD(pop)(vec, &vec->items[idx]);
}


#undef T
#undef SUFFIX

#ifdef DEFAULT_CAP
#   undef DEFAULT_CAP
#endif

