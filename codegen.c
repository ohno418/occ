#include "occ.h"

// Push the address to the stack.
void gen_addr(Node *node) {
  if (node->kind != ND_VAR) {
    fprintf(stderr, "expected var node: %d\n", node->kind);
    exit(1);
  }

  printf("    mov rax, rbp\n");
  printf("    sub rax, %d\n", node->offset);
  printf("    push rax\n");
}

// Push the result to the stack.
void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("    push %d\n", node->num);
    break;
  case ND_ADD:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    add rax, rdi\n");
    printf("    push rax\n");
    break;
  case ND_SUB:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    sub rax, rdi\n");
    printf("    push rax\n");
    break;
  case ND_MUL:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    imul rax, rdi\n");
    printf("    push rax\n");
    break;
  case ND_DIV:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    cqo\n");
    printf("    idiv rdi\n");
    printf("    push rax\n");
    break;
  case ND_VAR:
    gen_addr(node);
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n");
    printf("    push rax\n");
    break;
  case ND_ASSIGN:
    gen_addr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    mov [rax], rdi\n");
    printf("    push rax\n");
    break;
  default:
    fprintf(stderr, "unknown kind of expression node: %d\n", node->kind);
    exit(1);
  }
}

void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_STMT:
    gen_expr(node->lhs);
    printf("    pop rax\n");
    break;
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("    pop rax\n");
    printf("    jmp .Lend\n");
    break;
  default:
    fprintf(stderr, "unknown kind of statement node: %d\n", node->kind);
    exit(1);
  }
}

void prologue() {
  printf("    push rbp\n");
  printf("    mov rbp, rsp\n");
  // tmp: Up to 3 variables are available for now.
  printf("    sub rsp, 24\n");
}

void epilogue() {
  printf(".Lend:\n");
  printf("    mov rsp, rbp\n");
  printf("    pop rbp\n");
  printf("    ret\n");
}

void codegen(Node *node) {
  printf("    .intel_syntax noprefix\n");
  printf("    .globl main\n");

  printf("main:\n");
  prologue();
  for (Node *n = node; n; n = n->next)
    gen_stmt(n);
  epilogue();
}
