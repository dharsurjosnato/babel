#include "babel.h"
#include <stdlib.h>
#include <string.h>

void ast_free(Stmt *s) {
    while (s) {
        Stmt *next = s->next;
        free(s->name);
        Expr *stack[1024];
        size_t top = 0;
        if (s->expr) stack[top++] = s->expr;
        while (top) {
            Expr *e = stack[--top];
            if (e->left && top < 1024) stack[top++] = e->left;
            if (e->right && top < 1024) stack[top++] = e->right;
            free(e->name);
            free(e);
        }
        free(s);
        s = next;
    }
}
