#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
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
 * Slurp *
 *********/

#define check_errno(str) do { \
        if(errno) {           \
            perror(str);      \
            exit(1);          \
        }                     \
    } while (0)

long slurp(struct arena *a, char *filename, char **ptext) {
    errno = 0;
    FILE *file = fopen(filename, "r");
    check_errno(filename);
    assert(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = arena_push(a, size + 1);
    errno = 0;
    fread(buffer, size, 1, file);
    check_errno(NULL);
    fclose(file);
    buffer[size] = '\0';

    *ptext = buffer;
    return size;
}

/***************
 * Split Lines *
 ***************/

int split_lines(struct arena *a, char *text, char **lines[]) {
    char **items;
    size_t item_size = sizeof(items[0]);
    int capacity = INITIAL_CAPACITY;
    items = arena_push(a, capacity*item_size);

    char *save_ptr = NULL;
    int nb_items = 0;
    while (true) {
        if (capacity < nb_items) {
            arena_push(a, capacity*item_size); // extend the allocated size for items
            capacity += capacity;
        }
        char *token = strtok_r(text, "\n", &save_ptr);
        items[nb_items] = token;
        if (NULL == token) {
            break;
        }
        ++nb_items;
        text = NULL;
    }

    size_t excess = (capacity - nb_items)*item_size;
    arena_pop(a, excess); // trim the allocated size for items

    *lines = items;
    return nb_items;
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
