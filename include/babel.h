#ifndef BABEL_H
#define BABEL_H
#include <stddef.h>

typedef enum {
    TOK_EOF, TOK_NUMBER, TOK_IDENT, TOK_LET, TOK_PRINT,
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_TRUE, TOK_FALSE,
    TOK_ASSIGN, TOK_EQ, TOK_NE, TOK_LT, TOK_LE, TOK_GT, TOK_GE,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT, TOK_BANG,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, TOK_SEMICOLON,
    TOK_INVALID
} TokenType;
typedef struct { TokenType type; char *text; double number; size_t line, column; } Token;
typedef struct { Token *items; size_t count, capacity; } TokenArray;
typedef struct { char *canonical; char *spelling; } DictionaryEntry;
typedef struct { DictionaryEntry *items; size_t count, capacity; } Dictionary;
typedef enum { AST_NUMBER, AST_VARIABLE, AST_BINARY, AST_UNARY } ExprType;
typedef struct Expr Expr;
struct Expr { ExprType type; double number; char *name; TokenType op; Expr *left, *right; };
typedef enum { STMT_LET, STMT_ASSIGN, STMT_PRINT, STMT_IF, STMT_WHILE, STMT_FOR, STMT_BLOCK } StmtType;
typedef struct Stmt Stmt;
struct Stmt {
    StmtType type;
    char *name;
    Expr *expr;
    Stmt *body;      /* block body, or then-branch */
    Stmt *else_body; /* else-branch */
    Stmt *init;      /* for initialization */
    Stmt *update;    /* for update */
    Stmt *next;      /* next statement in same list */
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
