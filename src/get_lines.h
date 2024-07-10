#include <stdio.h>

#include "arena.h"

#define INITIAL_CAPACITY 8
#define MAX_LINES 2

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

int get_lines(struct arena *a, FILE *file, char **lines[]) {
    char **items;
    items = arena_push(a, MAX_LINES*sizeof(items[0]));

    char *line;
    int nb_lines = 0;
    while (0 != (line = get_line(a, file))) {
        assert(nb_lines < MAX_LINES);
        items[nb_lines++] = line;
    }
    *lines = items;
    return nb_lines;
}
