#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Arena
 */

struct arena {
    char *mem;
    size_t pos;
    size_t cap;
};

struct arena arena_of_size(size_t size) {
    assert(size);
    struct arena result = {0};
    result.mem = malloc(size);
    assert(result.mem);
    result.cap = size;
    return result;
}

void *arena_alloc(struct arena *a, size_t size) {
    assert(a->pos + size <= a->cap);

    void *result = a->mem + a->pos;
    a->pos += size;

    return result;
}

void arena_free(struct arena *a) {
    free(a->mem);
    *a = (struct arena){0};
}

size_t arena_size(struct arena *a) {
    return a->pos;
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

    struct arena scratch = *a;
    int capacity = INITIAL_CAPACITY;
    char *cstring = arena_alloc(&scratch, capacity);

    char c;
    int size = 0;
    while (EOF != (c = fgetc(file)) && c != '\n') {
        if (size + 1 >= capacity) {
            arena_alloc(&scratch, capacity);
            capacity += capacity;
        }
        cstring[size++] = c;
    };
    if (EOF == c && 0 == size) {
        return 0;
    }
    cstring[size] = '\0';
    arena_alloc(a, size + 1);

    struct string *result = arena_alloc(a, sizeof(struct string));
    result->cstring = cstring;
    result->size = size;

    return result;
}

void test_with_file() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_of_size(1000);

    // Calls & checks
    int nb_lines = 0;
    struct string *line;
    size_t expected_size = 0;
    while (0 != (line = get_line(&arena, file))) { // renvoyer une copie
        assert(strlen(line->cstring) == line->size);
        expected_size += line->size + 1 + sizeof(struct string);
        ++nb_lines;
    }

    assert(expected_size == arena_size(&arena));
    assert(2 == nb_lines);

    fclose(file);
    arena_free(&arena);
    assert(0 == arena.mem);
}

int main() {
    test_with_file();
    return 0;
}
