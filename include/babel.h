#ifndef BABEL_H
#define BABEL_H

#include <stddef.h>

typedef enum {
    TOK_EOF, TOK_NUMBER, TOK_IDENT,
    TOK_LET, TOK_PRINT,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_LPAREN, TOK_RPAREN, TOK_SEMICOLON,
    TOK_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    double number;
    size_t line;
    size_t column;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

typedef struct {
    char *canonical;
    char *spelling;
} DictionaryEntry;

typedef struct {
    DictionaryEntry *items;
    size_t count;
    size_t capacity;
} Dictionary;

typedef enum { AST_NUMBER, AST_VARIABLE, AST_BINARY, AST_UNARY } ExprType;
typedef struct Expr Expr;
struct Expr {
    ExprType type;
    double number;
    char *name;
    TokenType op;
    Expr *left;
    Expr *right;
};

typedef enum { STMT_LET, STMT_ASSIGN, STMT_PRINT } StmtType;
typedef struct Stmt Stmt;
struct Stmt {
    StmtType type;
    char *name;
    Expr *expr;
    Stmt *next;
};

int dictionary_load(Dictionary *dict, const char *path);
const char *dictionary_canonical(const Dictionary *dict, const char *spelling);
void dictionary_free(Dictionary *dict);

int lex_source(const char *source, const Dictionary *dict, TokenArray *out);
void tokens_free(TokenArray *tokens);

Stmt *parse_program(const TokenArray *tokens);
void ast_free(Stmt *program);

int interpret_program(const Stmt *program);
char *read_entire_file(const char *path);

#endif
