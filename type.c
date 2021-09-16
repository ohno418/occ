#include "occ.h"

Type *ty_char() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_CHAR;
  ty->size = 1;
  return ty;
}

Type *ty_int() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_INT;
  ty->size = 4;
  return ty;
}

Type *ty_ptr(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->size = 8;
  ty->base = base;
  return ty;
}

int size(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    return node->var->ty->size;
  case ND_DEREF:
    return size(node->lhs);
  default:
    fprintf(stderr, "unknown size of node: kind=%d\n", node->kind);
    exit(1);
  }
}
