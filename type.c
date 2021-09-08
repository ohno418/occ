#include "occ.h"

Type *ty_int() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_INT;
  ty->size = 8;
  return ty;
}

Type *ty_ptr(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->size = 8;
  ty->base = base;
  return ty;
}
