#include "occ.h"

Type *new_type(TypeKind kind, int size) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = kind;
  ty->size = size;
  return ty;
}

Type *ty_char() {
  return new_type(TY_CHAR, 1);
}

Type *ty_int() {
  return new_type(TY_INT, 4);
}

Type *ty_ptr(Type *base) {
  Type *ty = new_type(TY_PTR, 8);
  ty->base = base;
  return ty;
}
