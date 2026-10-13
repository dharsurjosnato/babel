#define _POSIX_C_SOURCE 200809L
#include "babel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *copy_string(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    return s;
}

int dictionary_load(Dictionary *dict, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return 0; }
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    while ((len = getline(&line, &cap, f)) >= 0) {
        (void)len;
        char *s = trim(line);
        if (!*s || *s == '#') continue;
        char *eq = strchr(s, '=');
        if (!eq) {
            fprintf(stderr, "%s: expected canonical=spelling\n", path);
            free(line); fclose(f); return 0;
        }
        *eq = '\0';
        char *canonical = trim(s);
        char *spelling = trim(eq + 1);
        if (!*canonical || !*spelling) {
            fprintf(stderr, "%s: empty dictionary field\n", path);
            free(line); fclose(f); return 0;
        }
        if (dict->count == dict->capacity) {
            size_t newcap = dict->capacity ? dict->capacity * 2 : 8;
            DictionaryEntry *p = realloc(dict->items, newcap * sizeof *p);
            if (!p) { free(line); fclose(f); return 0; }
            dict->items = p;
            dict->capacity = newcap;
        }
        dict->items[dict->count].canonical = copy_string(canonical);
        dict->items[dict->count].spelling = copy_string(spelling);
        if (!dict->items[dict->count].canonical || !dict->items[dict->count].spelling) {
            free(dict->items[dict->count].canonical);
            free(dict->items[dict->count].spelling);
            free(line); fclose(f); return 0;
        }
        dict->count++;
    }
    free(line);
    fclose(f);
    return 1;
}

const char *dictionary_canonical(const Dictionary *dict, const char *spelling) {
    for (size_t i = 0; i < dict->count; i++)
        if (strcmp(dict->items[i].spelling, spelling) == 0)
            return dict->items[i].canonical;
    return NULL;
}

void dictionary_free(Dictionary *dict) {
    for (size_t i = 0; i < dict->count; i++) {
        free(dict->items[i].canonical);
        free(dict->items[i].spelling);
    }
    free(dict->items);
    memset(dict, 0, sizeof *dict);
}
