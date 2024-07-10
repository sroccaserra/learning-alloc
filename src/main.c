#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "text.h"

#include "testing.h"

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

/*********
 * Tests *
 *********/

void test_get_line() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_alloc(128); // The following heap allocs fit in 128 bytes

    // Calls & checks
    int nb_lines = 0;
    char *line;
    size_t expected_used = 0;
    while (0 != (line = get_line(&arena, file))) {
        expected_used += strlen(line) + 1;
        ++nb_lines;
    }
    fclose(file);

    assert_equals(expected_used, arena_used(&arena));
    assert_equals(2, nb_lines);

    arena_free(&arena);
    assert_null(arena.mem);
}

void test_get_lines() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_alloc(128); // The following heap allocs fit in 128 bytes

    char **lines = NULL;
    int nb_lines = get_lines(&arena, file, &lines);
    fclose(file);

    assert_equals(2, nb_lines);
    assert_equals("A first line with a given length", lines[0]);
    assert_equals("A second line with a different length", lines[1]);

    arena_free(&arena);
}

void test_slurp() {
    struct arena arena = arena_alloc(128);

    char *text = NULL;
    long size = slurp(&arena, "input.txt", &text);

    assert_equals(71, size);
    assert_equals(71 + 1, arena_used(&arena));

    arena_free(&arena);
}

void test_split_lines() {
    struct arena arena = arena_alloc(64);
    char text[] = "two\nlines\n";

    char **lines = NULL;
    int nb_lines = split_lines(&arena, text, &lines);

    assert_equals(2, nb_lines);
    assert_equals("two", lines[0]);
    assert_equals("lines", lines[1]);
    assert_equals(nb_lines*sizeof(lines[0]), arena_used(&arena));

    arena_free(&arena);
}

int main() {
    test_get_line();
    test_get_lines();
    test_slurp();
    test_split_lines();
    return 0;
}
