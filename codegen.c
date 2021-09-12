#include "occ.h"

Function *cur_func;

int label_cnt = 0;

// regsiters for function arguments
char *arg_regs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// Push the address to the stack.
void gen_addr(Node *node) {
  if (node->kind != ND_VAR) {
    fprintf(stderr, "expected var node: %s\n", node->tok->loc);
    exit(1);
  }

  printf("    mov rax, rbp\n");
  printf("    sub rax, %d\n", node->var->offset);
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
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    add rax, rbx\n");
    printf("    push rax\n");
    break;
  case ND_SUB:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    sub rax, rbx\n");
    printf("    push rax\n");
    break;
  case ND_MUL:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    imul rax, rbx\n");
    printf("    push rax\n");
    break;
  case ND_DIV:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cqo\n");
    printf("    idiv rbx\n");
    printf("    push rax\n");
    break;
  case ND_EQ: {
    int label = label_cnt++;
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cmp rax, rbx\n");
    printf("    je .L.eq.%d\n", label);
    printf("    push 0\n");
    printf("    jmp .L.eq.end.%d\n", label);
    printf(".L.eq.%d:\n", label);
    printf("    push 1\n");
    printf(".L.eq.end.%d:\n", label);
    break;
  }
  case ND_NEQ: {
    int label = label_cnt++;
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cmp rax, rbx\n");
    printf("    jne .L.neq.%d\n", label);
    printf("    push 0\n");
    printf("    jmp .L.neq.end.%d\n", label);
    printf(".L.neq.%d:\n", label);
    printf("    push 1\n");
    printf(".L.neq.end.%d:\n", label);
    break;
  }
  case ND_LT: {
    int label = label_cnt++;
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cmp rax, rbx\n");
    printf("    jl .L.lt.true.%d\n", label);
    printf("    push 0\n");
    printf("    jmp .L.lt.end.%d\n", label);
    printf(".L.lt.true.%d:\n", label);
    printf("    push 1\n");
    printf(".L.lt.end.%d:\n", label);
    break;
  }
  case ND_LTE: {
    int label = label_cnt++;
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cmp rax, rbx\n");
    printf("    jle .L.lte.true.%d\n", label);
    printf("    push 0\n");
    printf("    jmp .L.lte.end.%d\n", label);
    printf(".L.lte.true.%d:\n", label);
    printf("    push 1\n");
    printf(".L.lte.end.%d:\n", label);
    break;
  }
  case ND_REF:
    gen_addr(node->lhs);
    break;
  case ND_DEREF:
    gen_expr(node->lhs);
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n");
    printf("    push rax\n");
    break;
  case ND_VAR:
    gen_addr(node);
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n");
    printf("    push rax\n");
    break;
  case ND_ASSIGN:
    gen_expr(node->rhs);
    gen_addr(node->lhs);
    printf("    pop rax\n");
    printf("    pop rbx\n");
    printf("    mov [rax], rbx\n");
    printf("    push rbx\n");
    break;
  case ND_FUNCALL:
    int i = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      if (!arg) {
        printf("d/0\n");
        break;
      }

      gen_expr(arg);
      printf("    pop rax\n");
      printf("    mov %s, rax\n", arg_regs[i]);
      i++;
    }

    printf("    call %s\n", node->func_name);
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
    printf("    jmp .L.end.%s\n", cur_func->name);
    break;
  case ND_IF: {
    int label = label_cnt++;
    gen_expr(node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .L.if.else.%d\n", label);
    gen_stmt(node->then);
    printf(".L.if.else.%d:\n", label);
    if (node->els)
      gen_stmt(node->els);
    break;
  }
  case ND_BLOCK:
    for (Node *stmt = node->body; stmt; stmt = stmt->next)
      gen_stmt(stmt);
    break;
  default:
    fprintf(stderr, "unknown kind of statement node: %d\n", node->kind);
    exit(1);
  }
}

void prologue(Var *lvars) {
  printf("    push rbp\n");
  printf("    mov rbp, rsp\n");

  // local variables
  int max_offset = 0;
  for (Var *v = lvars; v; v = v->next)
    if (max_offset < v->offset)
      max_offset = v->offset;
  printf("    sub rsp, %d\n", max_offset);

  // arguments
  int i = 0;
  for (Var *lv = cur_func->lvars; lv; lv = lv->next) {
    if (!lv->is_arg)
      continue;

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", lv->offset);
    printf("    mov [rax], %s\n", arg_regs[i]);
    i++;
  }
}

void epilogue() {
  printf(".L.end.%s:\n", cur_func->name);
  printf("    mov rsp, rbp\n");
  printf("    pop rbp\n");
  printf("    ret\n");
}

void codegen(Function *prog) {
  printf("    .intel_syntax noprefix\n");
  printf("    .globl main\n");

  for (Function *f = prog; f; f = f->next) {
    cur_func = f;

    printf("%s:\n", f->name);
    prologue(f->lvars);

    for (Node *n = f->body; n; n = n->next)
      gen_stmt(n);

    epilogue();
  }
}
