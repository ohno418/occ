#include "occ.h"

Type *ty_void() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_VOID;
  ty->size = 1;
  return ty;
}

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

Type *ty_array(Type *base, int num) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->size = base->size * num;
  ty->base = base;
  return ty;
}

Type *type_of(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    return node->var->ty;
  case ND_DEREF:
    return type_of(node->lhs)->base;
  case ND_ADD:
    return type_of(node->lhs);
  default:
    fprintf(stderr, "unknown size of node: kind=%d\n", node->kind);
    exit(1);
  }
}
