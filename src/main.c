#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "get_lines.h"
#include "text.h"

#include "testing.h"

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

void test_split_zero_lines() {
    struct arena arena = arena_alloc(64);
    char text[] = "";

    char **lines = NULL;
    int nb_lines = split_lines(&arena, text, &lines);

    assert_equals(0, nb_lines);
    assert_null(lines);

    arena_free(&arena);
}

void test_split_one_line() {
    struct arena arena = arena_alloc(64);
    char text[] = "one line\n";

    char **lines = NULL;
    int nb_lines = split_lines(&arena, text, &lines);

    assert_equals(1, nb_lines);
    assert_equals("one line", lines[0]);

    arena_free(&arena);
}

void test_split_one_line_without_eol() {
    struct arena arena = arena_alloc(64);
    char text[] = "one line";

    char **lines = NULL;
    int nb_lines = split_lines(&arena, text, &lines);

    assert_equals(1, nb_lines);
    assert_equals("one line", lines[0]);

    arena_free(&arena);
}

void test_split_two_lines() {
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

void test_split_two_lines_without_last_eol() {
    struct arena arena = arena_alloc(64);
    char text[] = "two\nlines";

    char **lines = NULL;
    int nb_lines = split_lines(&arena, text, &lines);

    assert_equals(2, nb_lines);
    assert_equals("two", lines[0]);
    assert_equals("lines", lines[1]);
    assert_equals(nb_lines*sizeof(lines[0]), arena_used(&arena));

    arena_free(&arena);
}

int main() {
    TEST_START("get_lines");
    test_get_line();
    test_get_lines();
    TEST_END;

    TEST_START("text");
    test_slurp();
    test_split_zero_lines();
    test_split_one_line();
    test_split_one_line_without_eol();
    test_split_two_lines();
    test_split_two_lines_without_last_eol();
    TEST_END;
    return 0;
}
