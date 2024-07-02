#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Arena
 */

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

void *arena_push(struct arena *a, size_t size) {
    assert(a->used + size <= a->size);

    void *result = a->mem + a->used;
    a->used += size;

    return result;
}

void arena_pop(struct arena *a, size_t size) {
    assert(size <= a->used);
    a->used -= size;
}

void arena_free(struct arena *a) {
    free(a->mem);
    *a = (struct arena){0};
}

size_t arena_used(struct arena *a) {
    return a->used;
}

/*
 * Text
 */

struct string {
    char *cstring;
    size_t size;
};

#define INITIAL_CAPACITY 8

struct string *get_line(struct arena *a, FILE *file) {
    assert(a);
    assert(file);
    if (feof(file)) {
        return 0;
    }

    char c;
    int size = 0;
    int capacity = INITIAL_CAPACITY;
    char *cstring = 0;
    while (EOF != (c = fgetc(file)) && c != '\n') {
        if (0 == cstring) {
            cstring = arena_push(a, capacity);
        }
        if (size + 1 >= capacity) {
            arena_push(a, capacity);
            capacity += capacity;
        }
        cstring[size++] = c;
    };
    if (EOF == c && 0 == size) {
        return 0;
    }
    cstring[size] = '\0';
    size_t excess = capacity - (size + 1);
    arena_pop(a, excess);

    struct string *result = arena_push(a, sizeof(struct string));
    result->cstring = cstring;
    result->size = size;

    return result;
}

void test_with_file() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_alloc(1000);

    // Calls & checks
    int nb_lines = 0;
    struct string *line;
    size_t expected_used = 0;
    while (0 != (line = get_line(&arena, file))) { // renvoyer une copie
        assert(strlen(line->cstring) == line->size);
        expected_used += line->size + 1 + sizeof(struct string);
        ++nb_lines;
    }

    assert(expected_used == arena_used(&arena));
    assert(2 == nb_lines);

    fclose(file);
    arena_free(&arena);
    assert(0 == arena.mem);
}

int main() {
    test_with_file();
    return 0;
}
