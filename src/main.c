#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct lines {
    char **items;
    int count;
};

struct lines *get_lines(struct arena *a, FILE *file) {
    int count = 0;
    char **items = arena_push(a, MAX_LINES*sizeof(*items));
    char *line;
    while (0 != (line = get_line(a, file))) {
        assert(count < MAX_LINES);
        items[count++] = line;
    }
    struct lines *result = arena_push(a, sizeof(*result));
    result->items = items;
    result->count = count;
    return result;
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

long slurp(struct arena *a, char *filename, char **ptr) {
    errno = 0;
    FILE *file = fopen(filename, "r");
    check_errno(filename);
    assert(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *text = arena_push(a, size + 1);
    errno = 0;
    fread(text, size, 1, file);
    check_errno(NULL);
    fclose(file);
    text[size] = '\0';

    *ptr = text;
    return size;
}

/***************
 * Split Lines *
 ***************/

void split_lines(struct arena *a, char *text, char **lines[], int *pnb_lines) {
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

    *pnb_lines = nb_items;
    *lines = items;
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

    assert(expected_used == arena_used(&arena));
    assert(2 == nb_lines);

    arena_free(&arena);
    assert(0 == arena.mem);
}

void test_get_lines() {
    FILE *file = fopen("input.txt", "r");
    assert(file);
    struct arena arena = arena_alloc(128); // The following heap allocs fit in 128 bytes
    struct lines *lines = get_lines(&arena, file);
    fclose(file);

    assert(2 == lines->count);
    assert(0 == strcmp("A first line with a given length", lines->items[0]));
    assert(0 == strcmp("A second line with a different length", lines->items[1]));

    arena_free(&arena);
}

void test_slurp() {
    struct arena arena = arena_alloc(128);

    char *text = NULL;
    long size = slurp(&arena, "input.txt", &text);

    assert(71 == size);
    assert(71+1 == arena_used(&arena));

    arena_free(&arena);
}

void test_split_lines() {
    struct arena arena = arena_alloc(64);
    char text[] = "two\nlines\n";

    char **lines = NULL;
    int nb_lines = 0;
    split_lines(&arena, text, &lines, &nb_lines);

    assert(2 == nb_lines);
    assert(0 == strcmp("two", lines[0]));
    assert(0 == strcmp("lines", lines[1]));
    assert(nb_lines*sizeof(lines[0]) == arena_used(&arena));

    arena_free(&arena);
}

int main() {
    test_get_line();
    test_get_lines();
    test_slurp();
    test_split_lines();
    return 0;
}
