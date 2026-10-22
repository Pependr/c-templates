// @https://raw.githubusercontent.com/Pependr/c-templates/refs/heads/master/deque.h
#ifndef DEQUE_H_
#define DEQUE_H_

#include <stdlib.h>
#include <stdbool.h>

#define DEQUE_CONCAT1(a, b) a##b
#define DEQUE_CONCAT2(a, b) DEQUE_CONCAT1(a, b)

#define DEQUE_NAME DEQUE_CONCAT2(deque_, SUFFIX)
#define DEQUE_TYPE DEQUE_CONCAT2(DEQUE_NAME, _s)
#define DEQUE_METHOD(name) DEQUE_CONCAT2(DEQUE_NAME, _##name)

#define DEQUE_SHRINK 0
#define DEQUE_GROW 1

#define deque__emod(a, b) ((((a) % (b)) + (b)) % (b))

#define deque_head(dq) ((dq)->items[deque__emod((dq)->off, (dq)->cap)])
#define deque_tail(dq) ((dq)->items[deque__emod(((dq)->off + (dq)->len - 1), (dq)->cap)])

#endif // DEQUE_H_


#ifndef T
#   error "T must be defined before including deque.h"
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
    long off;
    bool is_static;
} DEQUE_TYPE;


static inline bool DEQUE_METHOD(init)(DEQUE_TYPE *dq) {
    if (dq->items != NULL && dq->cap == 0)
        return false;

    if (dq->cap == 0)
#ifdef DEFAULT_CAP
        dq->cap = DEFAULT_CAP;
#else
        return false;
#endif

    if (dq->items == NULL) {
        dq->items = malloc(dq->cap * sizeof(T));
        if (!dq->items) return false;
        dq->len = 0;
        dq->off = 0;
    }

    return true;
}


static inline bool DEQUE_METHOD(_resize)(DEQUE_TYPE *dq, int grow) {
    size_t newcap = grow ? dq->cap * 2 : dq->cap / 2;

    T *new = malloc(newcap * sizeof(T));
    if (!new) return false;

    for (size_t i = 0; i < dq->len; ++i)
        new[i] = dq->items[deque__emod((i + dq->off), dq->cap)];

    free(dq->items);

    dq->items = new;
    dq->cap = newcap;
    dq->off = 0;
    
    return true;
}


static inline bool DEQUE_METHOD(push_right)(DEQUE_TYPE *dq, T item) {
    if (dq->len >= dq->cap && !dq->is_static)
        if (!DEQUE_METHOD(_resize)(dq, DEQUE_GROW)) return false;

    dq->items[deque__emod((dq->len++ + dq->off), dq->cap)] = item;

    return true;
}


static inline bool DEQUE_METHOD(push_left)(DEQUE_TYPE *dq, T item) {
    if (dq->len >= dq->cap && !dq->is_static)
        if (!DEQUE_METHOD(_resize)(dq, DEQUE_GROW)) return false;

    dq->items[deque__emod(--(dq->off), dq->cap)] = item;
    dq->len += 1;

    return true;
}


static inline bool DEQUE_METHOD(pop_right)(DEQUE_TYPE *dq, T *dest) {
    if (dq->len == 0) return false;

    if (dest) {
        *dest = dq->items[deque__emod((dq->off + --(dq->len)), dq->cap)];
    } else {
        dq->len -= 1;
    }

    if (dq->len != 0 && dq->cap / dq->len >= 2 && !dq->is_static)
        if (!DEQUE_METHOD(_resize)(dq, DEQUE_SHRINK)) return false;

    return true;
}


static inline bool DEQUE_METHOD(pop_left)(DEQUE_TYPE *dq, T *dest) {
    if (dq->len == 0) return false;

    if (dest) {
        *dest = dq->items[deque__emod(dq->off++, dq->cap)];
    } else {
        dq->off += 1;
    }

    dq->len -= 1;

    if (dq->len != 0 && dq->cap / dq->len >= 2 && !dq->is_static)
        if (!DEQUE_METHOD(_resize)(dq, DEQUE_SHRINK)) return false;

    return true;
}


#undef T
#undef SUFFIX

#ifdef DEFAULT_CAP
#   undef DEFAULT_CAP
#endif

