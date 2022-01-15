#include "occ.h"

void codegen(Node *node) {
  printf("  .intel_syntax noprefix\n");
  printf("  .text\n");
  printf("  .globl main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", node->num);
  printf("  ret\n");
  return;
}
