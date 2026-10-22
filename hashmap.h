#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdlib.h>
#include <stdbool.h>

#define HASHMAP_CONCAT1(a, b) a##b
#define HASHMAP_CONCAT2(a, b) HASHMAP_CONCAT1(a, b)

#define HASHMAP_NAME HASHMAP_CONCAT2(hashmap_, SUFFIX)
#define HASHMAP_TYPE HASHMAP_CONCAT2(HASHMAP_NAME, _s)
#define HASHMAP_BUCKET_TYPE HASHMAP_CONCAT2(HASHMAP_NAME, _bucket_s)
#define HASHMAP_ITER_TYPE HASHMAP_CONCAT2(HASHMAP_NAME, _iter_s)
#define HASHMAP_METHOD(name) HASHMAP_CONCAT2(HASHMAP_NAME, _##name)

static inline size_t hashmap_djb2(const char *s) {
    size_t hash = 5381;
    while (*s)
        hash = hash * 33 + *(s++);
    return hash;
}

#define hashmap_intcmp(a, b) ((a > b) - (a < b))

#endif // HASHMAP_H_


#ifdef STR_KEY
#   include <string.h>
#   define K const char*
#   define CMP_FN strcmp
#   define HASH_FN hashmap_djb2
#   undef STR_KEY
#endif


#ifndef K
#   error "K must be defined before including hashmap.h"
#endif

#ifndef V
#   error "V must be defined before including hashmap.h"
#endif

#ifndef CMP_FN
#   error "CMP_FN must be defined before including hashmap.h"
#endif

#ifndef HASH_FN
#   error "HASH_FN must be defined before including hashmap.h"
#endif


#ifndef SUFFIX
#   define SUFFIX HASHMAP_CONCAT2(HASHMAP_CONCAT2(K, _), V)
#endif

#if defined(DEFAULT_CAP) && DEFAULT_CAP <= 0
#   error "DEFAULT_CAP must be a positive integer"
#endif


typedef struct {
    K key;
    V value;
    bool is_set;
} HASHMAP_BUCKET_TYPE;

typedef struct {
    HASHMAP_BUCKET_TYPE *items;
    size_t len, cap;
    bool is_static;
} HASHMAP_TYPE;

typedef struct {
    K *key;
    V *value;
    size_t idx;
} HASHMAP_ITER_TYPE;


static inline bool HASHMAP_METHOD(init)(HASHMAP_TYPE *hm) {
    if (hm->items != NULL && hm->cap == 0)
        return false;

    if (hm->cap == 0)
#ifdef DEFAULT_CAP
        hm->cap = DEFAULT_CAP;
#else
        return false;
#endif

    if (hm->items == NULL) {
        hm->items = calloc(hm->cap, sizeof(HASHMAP_BUCKET_TYPE));
        if (!hm->items) return false;
        hm->len = 0;
    }

    return true;
}


static inline bool HASHMAP_METHOD(has)(const HASHMAP_TYPE *hm, K key) {
    HASHMAP_BUCKET_TYPE *bucket = &hm->items[HASH_FN(key) % hm->cap];

    return bucket->is_set && CMP_FN(key, bucket->key) == 0;
}


static inline bool HASHMAP_METHOD(get)(const HASHMAP_TYPE *hm, K key, V *dest) {
    if (!HASHMAP_METHOD(has)(hm, key)) return false;

    *dest = hm->items[HASH_FN(key) % hm->cap].value;

    return true;
}


static inline bool HASHMAP_METHOD(get_ptr)(const HASHMAP_TYPE *hm, K key, V **dest) {
    if (!HASHMAP_METHOD(has)(hm, key)) return false;

    *dest = &hm->items[HASH_FN(key) % hm->cap].value;

    return true;
}


static inline bool HASHMAP_METHOD(pop)(HASHMAP_TYPE *hm, K key, V *dest) {
    if (!HASHMAP_METHOD(has)(hm, key)) return false;

    HASHMAP_BUCKET_TYPE *bucket = &hm->items[HASH_FN(key) % hm->cap];
    
    bucket->is_set = false;
    hm->len -= 1;

    if (dest) {
        *dest = bucket->value;
    }

#ifdef ON_POP
    ON_POP(&bucket->key, &bucket->value);
#endif

    return true;
}


static inline bool HASHMAP_METHOD(iter)(const HASHMAP_TYPE *hm, HASHMAP_ITER_TYPE *it) {
    HASHMAP_BUCKET_TYPE *bucket;

    while (it->idx < hm->cap) {
        bucket = &hm->items[it->idx++];
        if (!bucket->is_set) continue;

        it->key   = &bucket->key;
        it->value = &bucket->value;

        return true;
    }

    return false;
}


static bool HASHMAP_METHOD(_grow)(HASHMAP_TYPE *hm) {
    size_t newcap = hm->cap * 2;
    HASHMAP_BUCKET_TYPE *new = calloc(newcap, sizeof(HASHMAP_BUCKET_TYPE));
    if (!new) return false;
    
    HASHMAP_BUCKET_TYPE *bucket;
    HASHMAP_ITER_TYPE it = {0};

    while (HASHMAP_METHOD(iter)(hm, &it)) {
        bucket = &new[HASH_FN(*it.key) % newcap];
        
        if (bucket->is_set) {
            newcap *= 2;
            it.idx  = 0;

            free(new);
            new = calloc(newcap, sizeof(HASHMAP_BUCKET_TYPE));
            if (!new) return false;

            continue;
        }

        bucket->key = *it.key;
        bucket->value = *it.value;
        bucket->is_set = true;
    }

    free(hm->items);
    hm->items = new;
    hm->cap = newcap;

    return true;
}


static inline bool HASHMAP_METHOD(set)(HASHMAP_TYPE *hm, K key, V value) { 
    HASHMAP_BUCKET_TYPE *bucket = &hm->items[HASH_FN(key) % hm->cap];

    while (bucket->is_set && CMP_FN(bucket->key, key) != 0) {
        if (hm->is_static) return false;
        if (!HASHMAP_METHOD(_grow)(hm)) return false;
        bucket = &hm->items[HASH_FN(key) % hm->cap];
    }

    bucket->key = key;
    bucket->value = value;
    bucket->is_set = true;

    return true;
}


#undef K
#undef V
#undef SUFFIX
#undef HASH_FN
#undef CMP_FN

#ifdef DEFAULT_CAP
#   undef DEFAULT_CAP
#endif

#ifdef ON_POP
#   undef ON_POP
#endif

