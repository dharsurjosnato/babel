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
        *out = -b; return 1;
    }
    if (!eval(e->left, env, &a) || !eval(e->right, env, &b)) return 0;
    switch (e->op) {
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
int interpret_program(const Stmt *program) {
    Environment env = {0}; int ok = 1;
    for (const Stmt *s = program; s && ok; s = s->next) {
        double value;
        if (!eval(s->expr, &env, &value)) { ok = 0; break; }
        if (s->type == STMT_PRINT) {
            if (fabs(value - round(value)) < 1e-10) printf("%.0f\n", value);
            else printf("%.10g\n", value);
        } else {
            ok = set_var(&env, s->name, value, s->type == STMT_LET);
        }
    }
    for (size_t i = 0; i < env.count; i++) free(env.items[i].name);
    free(env.items);
    return ok;
}
