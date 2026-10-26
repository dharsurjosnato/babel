#define _POSIX_C_SOURCE 200809L
#include "babel.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_range(const char *s, size_t n) {
    char *p = malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n); p[n] = '\0';
    return p;
}

static int push(TokenArray *a, Token t) {
    if (a->count == a->capacity) {
        size_t cap = a->capacity ? a->capacity * 2 : 32;
        Token *p = realloc(a->items, cap * sizeof *p);
        if (!p) return 0;
        a->items = p; a->capacity = cap;
    }
    a->items[a->count++] = t;
    return 1;
}

int lex_source(const char *s, const Dictionary *dict, TokenArray *out) {
    size_t i = 0, line = 1, col = 1;
    while (s[i]) {
        unsigned char c = (unsigned char)s[i];
        if (isspace(c)) {
            if (c == '\n') { line++; col = 1; } else col++;
            i++; continue;
        }
        if (c == '#') {
            while (s[i] && s[i] != '\n') { i++; col++; }
            continue;
        }
        Token t = {0}; t.line = line; t.column = col;
        if (isdigit(c)) {
            size_t start = i;
            while (isdigit((unsigned char)s[i])) { i++; col++; }
            if (s[i] == '.' && isdigit((unsigned char)s[i + 1])) {
                i++; col++;
                while (isdigit((unsigned char)s[i])) { i++; col++; }
            }
            t.type = TOK_NUMBER; t.text = copy_range(s + start, i - start);
            t.number = strtod(t.text ? t.text : "0", NULL);
        } else if (isalpha(c) || c == '_') {
            size_t start = i;
            while (isalnum((unsigned char)s[i]) || s[i] == '_') { i++; col++; }
            t.text = copy_range(s + start, i - start);
            const char *canonical = t.text ? dictionary_canonical(dict, t.text) : NULL;
            if (canonical && strcmp(canonical, "let") == 0) t.type = TOK_LET;
            else if (canonical && strcmp(canonical, "print") == 0) t.type = TOK_PRINT;
            else if (canonical && strcmp(canonical, "if") == 0) t.type = TOK_IF;
            else if (canonical && strcmp(canonical, "else") == 0) t.type = TOK_ELSE;
            else if (canonical && strcmp(canonical, "while") == 0) t.type = TOK_WHILE;
            else if (canonical && strcmp(canonical, "for") == 0) t.type = TOK_FOR;
            else if (canonical && strcmp(canonical, "true") == 0) t.type = TOK_TRUE;
            else if (canonical && strcmp(canonical, "false") == 0) t.type = TOK_FALSE;
            else t.type = TOK_IDENT;
        } else {
            i++; col++;
            if (s[i] == '=' && (c == '=' || c == '!' || c == '<' || c == '>')) {
                t.type = c == '=' ? TOK_EQ : c == '!' ? TOK_NE : c == '<' ? TOK_LE : TOK_GE;
                i++; col++;
                t.text = copy_range(s + i - 2, 2);
                if (!t.text || !push(out, t)) { free(t.text); return 0; }
                continue;
            }
            switch (c) {
                case '=': t.type = TOK_ASSIGN; break;
                case '<': t.type = TOK_LT; break;
                case '>': t.type = TOK_GT; break;
                case '!': t.type = TOK_BANG; break;
                case '{': t.type = TOK_LBRACE; break;
                case '}': t.type = TOK_RBRACE; break;
                case '+': t.type = TOK_PLUS; break;
                case '-': t.type = TOK_MINUS; break;
                case '*': t.type = TOK_STAR; break;
                case '/': t.type = TOK_SLASH; break;
                case '%': t.type = TOK_PERCENT; break;
                case '(': t.type = TOK_LPAREN; break;
                case ')': t.type = TOK_RPAREN; break;
                case ';': t.type = TOK_SEMICOLON; break;
                default:
                    fprintf(stderr, "%zu:%zu: unexpected character '%c'\n", line, col - 1, c);
                    return 0;
            }
            t.text = copy_range(s + i - 1, 1);
        }
        if (!t.text || !push(out, t)) { free(t.text); return 0; }
    }
    Token end = {0}; end.type = TOK_EOF; end.line = line; end.column = col;
    end.text = copy_range("", 0);
    if (!end.text || !push(out, end)) { free(end.text); return 0; }
    return 1;
}

void tokens_free(TokenArray *tokens) {
    for (size_t i = 0; i < tokens->count; i++) free(tokens->items[i].text);
    free(tokens->items);
    memset(tokens, 0, sizeof *tokens);
}
