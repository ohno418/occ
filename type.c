#include "occ.h"

Type *ty_int() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_INT;
  ty->size = 8;
  return ty;
}
