#include "babel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    char *buf = malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, (size_t)size, f);
    if (ferror(f)) { free(buf); fclose(f); return NULL; }
    buf[n] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 4 || strcmp(argv[1], "--lang") != 0) {
        fprintf(stderr, "Usage: %s --lang <language-code> <source-file>\n", argv[0]);
        return 2;
    }
    char dict_path[256];
    int n = snprintf(dict_path, sizeof dict_path, "languages/%s.dict", argv[2]);
    if (n < 0 || (size_t)n >= sizeof dict_path) {
        fprintf(stderr, "Language code is too long.\n");
        return 2;
    }

    Dictionary dict = {0};
    if (!dictionary_load(&dict, dict_path)) {
        fprintf(stderr, "Could not load language pack: %s\n", dict_path);
        dictionary_free(&dict);
        return 1;
    }
    char *source = read_entire_file(argv[3]);
    if (!source) { dictionary_free(&dict); return 1; }

    TokenArray tokens = {0};
    int ok = lex_source(source, &dict, &tokens);
    free(source);
    if (!ok) {
        tokens_free(&tokens);
        dictionary_free(&dict);
        return 1;
    }

    Stmt *program = parse_program(&tokens);
    if (!program && tokens.count > 1) {
        tokens_free(&tokens);
        dictionary_free(&dict);
        return 1;
    }
    ok = interpret_program(program);
    ast_free(program);
    tokens_free(&tokens);
    dictionary_free(&dict);
    return ok ? 0 : 1;
}
