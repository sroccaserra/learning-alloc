#ifndef TEXT_H
#define TEXT_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"

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
    if ('\0' == text[0]) {
        *lines = NULL;
        return 0;
    }

    char **items;
    size_t item_size = sizeof(items[0]);
    int capacity = 8;
    items = arena_push(a, capacity*item_size);

    char *save_ptr = NULL;
    int nb_items = 0;
    while (1) {
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

#endif
