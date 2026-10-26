#include "babel.h"
#include <stdlib.h>
static void expr_free(Expr *e) {
    if (!e) return;
    expr_free(e->left); expr_free(e->right);
    free(e->name); free(e);
}
void ast_free(Stmt *s) {
    while (s) {
        Stmt *next = s->next;
        free(s->name);
        expr_free(s->expr);
        ast_free(s->body);
        ast_free(s->else_body);
        ast_free(s->init);
        ast_free(s->update);
        free(s);
        s = next;
    }
}
