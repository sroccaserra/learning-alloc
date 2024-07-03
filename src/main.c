#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

/*
 * Text
 */

#define INITIAL_CAPACITY 8

char *get_line(struct arena *a, FILE *file) {
    assert(a);
    assert(file);
    if (feof(file)) {
        return 0;
    }

    char c;
    int size = 0;
    int capacity = INITIAL_CAPACITY;
    char *result = 0;
    while (EOF != (c = fgetc(file)) && c != '\n') {
        if (0 == result) {
            result = arena_push(a, capacity);
        }
        if (size + 1 >= capacity) {
            arena_push(a, capacity);
            capacity += capacity;
        }
        result[size++] = c;
    };
    if (EOF == c && 0 == size) {
        return 0;
    }
    result[size] = '\0';
    size_t excess = capacity - (size + 1);
    arena_pop(a, excess);

    return result;
}

struct lines {
    char **items;
    int count;
};

struct lines *get_lines(struct arena *a, FILE *file) {
    int nb_lines = 0;
    char *c = arena_pointer(a);
    while (0 != get_line(a, file)) {
        ++nb_lines;
    }
    char **items = arena_push(a, nb_lines*sizeof(char *));
    for (int i = 0 ; i < nb_lines; ++i) {
        char *line = c;
        while(*c != '\0') {
            ++c;
        }
        ++c;
        items[i] = line;
    }
    struct lines *result = arena_push(a, sizeof(struct lines));
    result->items = items;
    result->count = nb_lines;
    return result;
}

void test_get_line() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_alloc(1000);

    // Calls & checks
    int nb_lines = 0;
    char *line;
    size_t expected_used = 0;
    while (0 != (line = get_line(&arena, file))) {
        expected_used += strlen(line) + 1;
        ++nb_lines;
    }
    fclose(file);

    assert(expected_used == arena_used(&arena));
    assert(2 == nb_lines);

    arena_free(&arena);
    assert(0 == arena.mem);
}

void test_get_lines() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_alloc(1000);
    struct lines *lines = get_lines(&arena, file);
    fclose(file);

    assert(2 == lines->count);
    assert(0 == strcmp("A first line with a given length", lines->items[0]));
    assert(0 == strcmp("A second line with a different length", lines->items[1]));

    arena_free(&arena);
}

int main() {
    test_get_line();
    test_get_lines();
    return 0;
}
