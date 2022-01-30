#include "occ.h"

Function *current_func;

int label_counter = 0;

// Push an address of a lvalue on stack top.
void gen_addr(Node *node) {
  if (!node->var) {
    fprintf(stderr, "not a lvalue\n");
    exit(1);
  }

  printf("  lea rax, [rbp - %d]\n", node->var->offset);
  printf("  push rax\n");
}

// Push a result on stack top.
void gen_expr(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->num);
      return;
    case ND_VAR:
      gen_addr(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
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
    case ND_DIV:
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  cqo\n");
      printf("  idiv rdi\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_addr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_FUNCALL:
      printf("  call %s\n", node->func_name);
      printf("  push rax\n");
      return;
    case ND_COMMA:
      gen_expr(node->lhs);
      printf("  pop rax\n");
      gen_expr(node->rhs);
      return;
    default:
      fprintf(stderr, "unknown kind of expr node: %d\n", node->kind);
      exit(1);
  }

  assert(0);
}

void gen_stmt(Node *node) {
  switch (node->kind) {
    case ND_EXPR_STMT:
      gen_expr(node->body);
      printf("  pop rax\n");
      return;
    case ND_BLOCK:
      for (Node *s = node->body; s; s = s->next)
        gen_stmt(s);
      return;
    case ND_IF: {
      int cnt = ++label_counter;
      gen_expr(node->cond);
      printf("  pop rax\n");
      printf("  test rax, rax\n");
      printf("  jz .L.if.%d.else\n", cnt);
      gen_stmt(node->body);
      printf(".L.if.%d.else:\n", cnt);
      if (node->els)
        gen_stmt(node->els);
      return;
    }
    case ND_RETURN:
      gen_expr(node->body);
      printf("  pop rax\n");
      printf("  jmp .L.%s.end\n", current_func->name);
      return;
    case ND_NULL_STMT:
      return;
    default:
      fprintf(stderr, "unknown kind of statement node: %d\n", node->kind);
      exit(1);
  }

  assert(0);
}

// Assign offset to local variables and stack area for them.
void assign_lvar_offset(Var *vars) {
  int offset = 0;
  for (Var *v = vars; v; v = v->next) {
    offset += 8;
    v->offset = offset;
  }
  printf("  sub rsp, %d\n", offset);
}

void codegen(Function *func) {
  printf("  .intel_syntax noprefix\n");
  printf("  .text\n");
  printf("  .globl main\n");

  for (current_func = func; current_func; current_func = current_func->next) {
    printf("%s:\n", current_func->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    assign_lvar_offset(current_func->vars);

    for (Node *node = current_func->body; node; node = node->next)
      gen_stmt(node);

    printf(".L.%s.end:\n", current_func->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}
