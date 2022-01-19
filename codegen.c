#include "occ.h"

// Push a result on stack top.
void gen_expr(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->num);
      return;
    case ND_ADD:
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  add rax, rdi\n");
      printf("  push rax\n");
      return;
    case ND_SUB:
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  sub rax, rdi\n");
      printf("  push rax\n");
      return;
    case ND_MUL:
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  imul rax, rdi\n");
      printf("  push rax\n");
      return;
    default:
      fprintf(stderr, "unknown kind of expr node: %d\n", node->kind);
      exit(1);
  }
}

void codegen(Node *node) {
  printf("  .intel_syntax noprefix\n");
  printf("  .text\n");
  printf("  .globl main\n");
  printf("main:\n");
  gen_expr(node);
  printf("  pop rax\n");
  printf("  ret\n");
}
