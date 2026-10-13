#include "babel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const TokenArray *tokens; size_t pos; int failed; } Parser;
static Token *cur(Parser *p) { return &p->tokens->items[p->pos]; }
static Token *take(Parser *p) { return &p->tokens->items[p->pos++]; }
static void error(Parser *p, const char *message) {
    Token *t = cur(p);
    fprintf(stderr, "%zu:%zu: %s near '%s'\n", t->line, t->column, message, t->text);
    p->failed = 1;
}
static char *dupstr(const char *s) {
    size_t n = strlen(s) + 1; char *p = malloc(n); if (p) memcpy(p, s, n); return p;
}
static Expr *new_expr(ExprType type) {
    Expr *e = calloc(1, sizeof *e); if (e) e->type = type; return e;
}
static Expr *expression(Parser *p);
static Expr *primary(Parser *p) {
    Token *t = cur(p);
    if (t->type == TOK_NUMBER) {
        Expr *e = new_expr(AST_NUMBER); if (!e) return NULL;
        e->number = take(p)->number; return e;
    }
    if (t->type == TOK_IDENT) {
        Expr *e = new_expr(AST_VARIABLE); if (!e) return NULL;
        e->name = dupstr(take(p)->text); return e;
    }
    if (t->type == TOK_LPAREN) {
        take(p); Expr *e = expression(p);
        if (cur(p)->type != TOK_RPAREN) error(p, "expected ')'");
        else take(p);
        return e;
    }
    if (t->type == TOK_MINUS) {
        take(p); Expr *e = new_expr(AST_UNARY); if (!e) return NULL;
        e->op = TOK_MINUS; e->right = primary(p); return e;
    }
    error(p, "expected an expression"); return NULL;
}
static Expr *factor(Parser *p) {
    Expr *left = primary(p);
    while (!p->failed && (cur(p)->type == TOK_STAR || cur(p)->type == TOK_SLASH || cur(p)->type == TOK_PERCENT)) {
        TokenType op = take(p)->type; Expr *right = primary(p);
        Expr *e = new_expr(AST_BINARY); if (!e) return NULL;
        e->op = op; e->left = left; e->right = right; left = e;
    }
    return left;
}
static Expr *expression(Parser *p) {
    Expr *left = factor(p);
    while (!p->failed && (cur(p)->type == TOK_PLUS || cur(p)->type == TOK_MINUS)) {
        TokenType op = take(p)->type; Expr *right = factor(p);
        Expr *e = new_expr(AST_BINARY); if (!e) return NULL;
        e->op = op; e->left = left; e->right = right; left = e;
    }
    return left;
}
static int expect(Parser *p, TokenType type, const char *message) {
    if (cur(p)->type != type) { error(p, message); return 0; }
    take(p); return 1;
}
Stmt *parse_program(const TokenArray *tokens) {
    Parser p = {tokens, 0, 0}; Stmt *head = NULL, **tail = &head;
    while (cur(&p)->type != TOK_EOF && !p.failed) {
        Stmt *s = calloc(1, sizeof *s);
        if (!s) { p.failed = 1; break; }
        if (cur(&p)->type == TOK_LET) {
            take(&p); s->type = STMT_LET;
            if (cur(&p)->type != TOK_IDENT) error(&p, "expected variable name");
            else s->name = dupstr(take(&p)->text);
            expect(&p, TOK_ASSIGN, "expected '=' after variable name");
            s->expr = expression(&p);
        } else if (cur(&p)->type == TOK_PRINT) {
            take(&p); s->type = STMT_PRINT; s->expr = expression(&p);
        } else if (cur(&p)->type == TOK_IDENT) {
            s->type = STMT_ASSIGN; s->name = dupstr(take(&p)->text);
            expect(&p, TOK_ASSIGN, "expected '=' in assignment");
            s->expr = expression(&p);
        } else {
            error(&p, "expected 'let', 'print', or assignment");
        }
        if (!p.failed) expect(&p, TOK_SEMICOLON, "expected ';' after statement");
        if (p.failed || !s->expr || ((s->type != STMT_PRINT) && !s->name)) {
            free(s->name); free(s); break;
        }
        *tail = s; tail = &s->next;
    }
    if (p.failed) { ast_free(head); return NULL; }
    return head;
}
