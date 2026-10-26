#include "babel.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { char *name; double value; } Variable;
typedef struct { Variable *items; size_t count, capacity; } Environment;

static int find_var(Environment *env, const char *name) {
    for (size_t i = 0; i < env->count; i++)
        if (strcmp(env->items[i].name, name) == 0) return (int)i;
    return -1;
}
static int set_var(Environment *env, const char *name, double value, int declare) {
    int i = find_var(env, name);
    if (declare && i >= 0) { fprintf(stderr, "variable '%s' is already declared\n", name); return 0; }
    if (!declare && i < 0) { fprintf(stderr, "undefined variable '%s'\n", name); return 0; }
    if (i >= 0) { env->items[i].value = value; return 1; }
    if (env->count == env->capacity) {
        size_t cap = env->capacity ? env->capacity * 2 : 16;
        Variable *p = realloc(env->items, cap * sizeof *p);
        if (!p) return 0;
        env->items = p; env->capacity = cap;
    }
    size_t n = strlen(name) + 1;
    char *copy = malloc(n); if (!copy) return 0; memcpy(copy, name, n);
    env->items[env->count++] = (Variable){copy, value};
    return 1;
}
static int eval(const Expr *e, Environment *env, double *out) {
    if (!e) return 0;
    if (e->type == AST_NUMBER) { *out = e->number; return 1; }
    if (e->type == AST_VARIABLE) {
        int i = find_var(env, e->name);
        if (i < 0) { fprintf(stderr, "undefined variable '%s'\n", e->name); return 0; }
        *out = env->items[i].value; return 1;
    }
    double a, b;
    if (e->type == AST_UNARY) {
        if (!eval(e->right, env, &b)) return 0;
        *out = e->op == TOK_BANG ? (b == 0.0) : -b; return 1;
    }
    if (!eval(e->left, env, &a) || !eval(e->right, env, &b)) return 0;
    switch (e->op) {
        case TOK_EQ: *out = a == b; break;
        case TOK_NE: *out = a != b; break;
        case TOK_LT: *out = a < b; break;
        case TOK_LE: *out = a <= b; break;
        case TOK_GT: *out = a > b; break;
        case TOK_GE: *out = a >= b; break;
        case TOK_PLUS: *out = a + b; break;
        case TOK_MINUS: *out = a - b; break;
        case TOK_STAR: *out = a * b; break;
        case TOK_SLASH:
            if (b == 0.0) { fprintf(stderr, "division by zero\n"); return 0; }
            *out = a / b; break;
        case TOK_PERCENT:
            if (b == 0.0) { fprintf(stderr, "remainder by zero\n"); return 0; }
            *out = fmod(a, b); break;
        default: return 0;
    }
    return 1;
}
/* The initial version uses one shared environment, including for nested blocks.
 * This preserves existing variable behavior; lexical scopes are a later feature. */
static int run(const Stmt *program, Environment *env);
static int run_one(const Stmt *s, Environment *env) {
    double value = 0;
    switch (s->type) {
        case STMT_BLOCK:
            return run(s->body, env);
        case STMT_IF:
            if (!eval(s->expr, env, &value)) return 0;
            return value != 0 ? run(s->body, env) : run(s->else_body, env);
        case STMT_WHILE:
            for (;;) {
                if (!eval(s->expr, env, &value)) return 0;
                if (value == 0) return 1;
                if (!run(s->body, env)) return 0;
            }
        case STMT_FOR:
            if (s->init && !run_one(s->init, env)) return 0;
            for (;;) {
                if (s->expr) {
                    if (!eval(s->expr, env, &value)) return 0;
                    if (value == 0) return 1;
                }
                if (!run(s->body, env)) return 0;
                if (s->update && !run_one(s->update, env)) return 0;
            }
        case STMT_LET:
        case STMT_ASSIGN:
        case STMT_PRINT:
            if (!eval(s->expr, env, &value)) return 0;
            if (s->type == STMT_PRINT) {
                if (fabs(value - round(value)) < 1e-10) printf("%.0f\n", value);
                else printf("%.10g\n", value);
                return 1;
            }
            return set_var(env, s->name, value, s->type == STMT_LET);
    }
    return 0;
}
static int run(const Stmt *program, Environment *env) {
    for (const Stmt *s = program; s; s = s->next)
        if (!run_one(s, env)) return 0;
    return 1;
}
int interpret_program(const Stmt *program) {
    Environment env = {0};
    int ok = run(program, &env);
    for (size_t i = 0; i < env.count; i++) free(env.items[i].name);
    free(env.items);
    return ok;
}
