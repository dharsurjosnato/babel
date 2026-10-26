#include "babel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const TokenArray *tokens; size_t pos; int failed; } Parser;
static const Token *cur(Parser *p) { return &p->tokens->items[p->pos]; }
static const Token *take(Parser *p) { const Token *t = cur(p); if (t->type != TOK_EOF) p->pos++; return t; }
static void error(Parser *p, const char *message) {
    if (!p->failed) fprintf(stderr, "%zu:%zu: %s near '%s'\n", cur(p)->line, cur(p)->column, message, cur(p)->text);
    p->failed = 1;
}
static char *dupstr(const char *s) { size_t n = strlen(s) + 1; char *p = malloc(n); if (p) memcpy(p, s, n); return p; }
static Expr *new_expr(ExprType type) { Expr *e = calloc(1, sizeof *e); if (e) e->type = type; return e; }
static Stmt *new_stmt(StmtType type) { Stmt *s = calloc(1, sizeof *s); if (s) s->type = type; return s; }
static int expect(Parser *p, TokenType type, const char *message) {
    if (cur(p)->type != type) { error(p, message); return 0; } take(p); return 1;
}
static void free_expr(Expr *e) {
    if (!e) return;
    free_expr(e->left); free_expr(e->right); free(e->name); free(e);
}
static Expr *expression(Parser *p);
static Expr *primary(Parser *p) {
    const Token *t = cur(p);
    if (t->type == TOK_NUMBER || t->type == TOK_TRUE || t->type == TOK_FALSE) {
        Expr *e = new_expr(AST_NUMBER); if (!e) { error(p, "out of memory"); return NULL; }
        e->number = t->type == TOK_TRUE ? 1 : t->type == TOK_FALSE ? 0 : t->number; take(p); return e;
    }
    if (t->type == TOK_IDENT) {
        Expr *e = new_expr(AST_VARIABLE); if (!e) { error(p, "out of memory"); return NULL; }
        e->name = dupstr(take(p)->text); if (!e->name) error(p, "out of memory"); return e;
    }
    if (t->type == TOK_LPAREN) {
        take(p); Expr *e = expression(p); expect(p, TOK_RPAREN, "expected ')'"); return e;
    }
    error(p, "expected an expression"); return NULL;
}
static Expr *unary(Parser *p) {
    if (cur(p)->type == TOK_MINUS || cur(p)->type == TOK_BANG) {
        TokenType op = take(p)->type;
        Expr *e = new_expr(AST_UNARY); if (!e) { error(p, "out of memory"); return NULL; }
        e->op = op; e->right = unary(p); return e;
    }
    return primary(p);
}
static Expr *binary_level(Parser *p, int level) {
    if (level == 0) return unary(p);
    Expr *left = binary_level(p, level - 1);
    while (!p->failed) {
        TokenType op = cur(p)->type;
        int match = level == 1 ? (op == TOK_STAR || op == TOK_SLASH || op == TOK_PERCENT)
            : level == 2 ? (op == TOK_PLUS || op == TOK_MINUS)
            : level == 3 ? (op == TOK_LT || op == TOK_LE || op == TOK_GT || op == TOK_GE)
            : (op == TOK_EQ || op == TOK_NE);
        if (!match) break;
        take(p); Expr *right = binary_level(p, level - 1);
        Expr *e = new_expr(AST_BINARY);
        if (!e) { error(p, "out of memory"); free_expr(right); break; }
        e->op = op; e->left = left; e->right = right; left = e;
    }
    return left;
}
static Expr *expression(Parser *p) { return binary_level(p, 4); }
static Stmt *statement(Parser *p);
static Stmt *statement_list(Parser *p, int braced) {
    Stmt *head = NULL, **tail = &head;
    while (!p->failed && cur(p)->type != TOK_EOF && (!braced || cur(p)->type != TOK_RBRACE)) {
        Stmt *s = statement(p);
        if (!s) break;
        *tail = s; tail = &s->next;
    }
    if (braced && !p->failed) expect(p, TOK_RBRACE, "expected '}'");
    if (p->failed) { ast_free(head); return NULL; }
    return head;
}
static Stmt *simple_statement(Parser *p, int semicolon) {
    TokenType type = cur(p)->type;
    StmtType kind = type == TOK_LET ? STMT_LET : type == TOK_PRINT ? STMT_PRINT : STMT_ASSIGN;
    if (type != TOK_LET && type != TOK_PRINT && type != TOK_IDENT) { error(p, "expected declaration, assignment, or print"); return NULL; }
    Stmt *s = new_stmt(kind); if (!s) { error(p, "out of memory"); return NULL; }
    if (type == TOK_LET || type == TOK_PRINT) take(p);
    if (kind != STMT_PRINT) {
        if (cur(p)->type != TOK_IDENT) error(p, "expected variable name");
        else { s->name = dupstr(take(p)->text); if (!s->name) error(p, "out of memory"); }
        if (!p->failed) expect(p, TOK_ASSIGN, "expected '=' after variable name");
    }
    if (!p->failed) s->expr = expression(p);
    if (!p->failed && semicolon) expect(p, TOK_SEMICOLON, "expected ';' after statement");
    if (p->failed || !s->expr) { ast_free(s); return NULL; }
    return s;
}
static Stmt *statement(Parser *p) {
    TokenType type = cur(p)->type;
    if (type == TOK_LBRACE) {
        take(p); Stmt *s = new_stmt(STMT_BLOCK);
        if (!s) { error(p, "out of memory"); return NULL; }
        s->body = statement_list(p, 1);
        if (p->failed) { ast_free(s); return NULL; }
        return s;
    }
    if (type == TOK_IF || type == TOK_WHILE) {
        take(p); Stmt *s = new_stmt(type == TOK_IF ? STMT_IF : STMT_WHILE);
        if (!s) { error(p, "out of memory"); return NULL; }
        expect(p, TOK_LPAREN, "expected '('");
        if (!p->failed) s->expr = expression(p);
        if (!p->failed) expect(p, TOK_RPAREN, "expected ')'");
        if (!p->failed) s->body = statement(p);
        if (!p->failed && type == TOK_IF && cur(p)->type == TOK_ELSE) { take(p); s->else_body = statement(p); }
        if (p->failed || !s->expr || !s->body) { ast_free(s); return NULL; }
        return s;
    }
    if (type == TOK_FOR) {
        take(p); Stmt *s = new_stmt(STMT_FOR);
        if (!s) { error(p, "out of memory"); return NULL; }
        expect(p, TOK_LPAREN, "expected '(' after for");
        if (!p->failed && cur(p)->type != TOK_SEMICOLON) s->init = simple_statement(p, 0);
        if (!p->failed) expect(p, TOK_SEMICOLON, "expected ';' after for initializer");
        if (!p->failed && cur(p)->type != TOK_SEMICOLON) s->expr = expression(p);
        if (!p->failed) expect(p, TOK_SEMICOLON, "expected ';' after for condition");
        if (!p->failed && cur(p)->type != TOK_RPAREN) s->update = simple_statement(p, 0);
        if (!p->failed) expect(p, TOK_RPAREN, "expected ')' after for clauses");
        if (!p->failed) s->body = statement(p);
        if (p->failed || !s->body) { ast_free(s); return NULL; }
        return s;
    }
    return simple_statement(p, 1);
}
Stmt *parse_program(const TokenArray *tokens) {
    Parser p = {tokens, 0, 0};
    return statement_list(&p, 0);
}
