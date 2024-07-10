#ifndef ARENA_H
#define ARENA_H

#include <assert.h>
#include <stdlib.h>

struct arena {
    char *mem;
    size_t used;
    size_t size;
};

struct arena arena_alloc(size_t size) {
    assert(size);
    char *mem = malloc(size);
    assert(mem);
    return (struct arena){.mem = mem, .size = size};
}

void arena_free(struct arena *a) {
    free(a->mem);
    *a = (struct arena){0};
}

#define arena_pointer(a) ((a)->mem + (a)->used)

void *arena_push(struct arena *a, size_t size) {
    assert(a->used + size <= a->size);

    void *result = arena_pointer(a);
    a->used += size;

    return result;
}

void arena_pop(struct arena *a, size_t size) {
    assert(size <= a->used);
    a->used -= size;
}

size_t arena_used(struct arena *a) {
    return a->used;
}

#endif
