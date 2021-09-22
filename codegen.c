#include "occ.h"

Function *cur_func;

int label_cnt = 0;

// regsiters for function arguments
char *arg_64_regs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *arg_32_regs[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

// Load a value from where the stack top is pointing to.
void load(Type *ty) {
  if (ty->kind == TY_ARRAY)
    return;

  printf("    pop rax\n");
  switch (ty->size) {
  case 1:
    printf("    movzx rax, BYTE PTR [rax]\n");
    break;
  case 4:
    printf("    mov eax, [rax]\n");
    break;
  case 8:
    printf("    mov rax, [rax]\n");
    break;
  default:
    fprintf(stderr, "unknown size of variable: TypeKind=%d, Size=%d\n", ty->kind, ty->size);
    exit(1);
  }
  printf("    push rax\n");
}

// Store RAX value to an address of the stack top.
void store(Type *ty) {
  printf("    pop rbx\n");
  switch (ty->size) {
  case 1:
    printf("    mov BYTE PTR [rbx], al\n");
    break;
  case 4:
    printf("    mov [rbx], eax\n");
    break;
  case 8:
    printf("    mov [rbx], rax\n");
    break;
  default:
    fprintf(stderr, "unknown size of variable: TypeKind=%d, Size=%d\n", ty->kind, ty->size);
    exit(1);
  }
  printf("    push rax\n");
}

void gen_expr(Node *node);

// Push the address to the stack.
void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    if (node->var->is_global) {
      printf("    lea rax, %s[rip]\n", node->var->name);
      printf("    push rax\n");
    } else {
      printf("    lea rax, -%d[rbp]\n", node->var->offset);
      printf("    push rax\n");
    }
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  default:
    fprintf(stderr, "not assignable variable: %s\n", node->tok->loc);
    exit(1);
  }
}

// Push the result to the stack.
void gen_expr(Node *node) {
  if (node == NULL) {
    printf("    push 0\n");
    return;
  }

  switch (node->kind) {
  case ND_NUM:
    printf("    push %d\n", node->num);
    break;
  case ND_CHAR:
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
  case ND_AND: {
    int label = label_cnt++;
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .L.and.false.%d\n", label);
    printf("    cmp rbx, 0\n");
    printf("    je .L.and.false.%d\n", label);
    printf("    push 1\n");
    printf("    jmp .L.and.end.%d\n", label);
    printf(".L.and.false.%d:\n", label);
    printf("    push 0\n");
    printf(".L.and.end.%d:\n", label);
    break;
  }
  case ND_OR: {
    int label = label_cnt++;
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rbx\n");
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    jne .L.or.true.%d\n", label);
    printf("    cmp rbx, 0\n");
    printf("    jne .L.or.true.%d\n", label);
    printf("    push 0\n");
    printf("    jmp .L.or.end.%d\n", label);
    printf(".L.or.true.%d:\n", label);
    printf("    push 1\n");
    printf(".L.or.end.%d:\n", label);
    break;
  }
  case ND_REF:
    gen_addr(node->lhs);
    break;
  case ND_DEREF:
    gen_expr(node->lhs);
    load(type_of(node));
    break;
  case ND_VAR:
    gen_addr(node);
    load(type_of(node));
    break;
  case ND_ASSIGN:
    gen_addr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rax\n");
    store(type_of(node->lhs));
    break;
  case ND_FUNCALL:
    int i = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      if (!arg) {
        break;
      }

      gen_expr(arg);
      printf("    pop rax\n");
      printf("    mov %s, rax\n", arg_64_regs[i]);
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
  case ND_FOR: {
    int label = label_cnt++;
    gen_expr(node->init);
    printf("    pop rax\n");
    printf(".L.for.cond.%d:\n", label);
    gen_expr(node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .L.for.end.%d\n", label);
    gen_stmt(node->then);
    gen_expr(node->inc);
    printf("    pop rax\n");
    printf("    jmp .L.for.cond.%d\n", label);
    printf(".L.for.end.%d:\n", label);
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
    if (lv->ty->size == 4)
      printf("    mov [rax], %s\n", arg_32_regs[i]);
    else
      printf("    mov [rax], %s\n", arg_64_regs[i]);
    i++;
  }
}

void epilogue() {
  printf(".L.end.%s:\n", cur_func->name);
  printf("    mov rsp, rbp\n");
  printf("    pop rbp\n");
  printf("    ret\n");
}

void emit_data() {
  printf("    .data\n");
  for (Var *g = gvars; g; g = g->next) {
    printf("    .globl %s\n", g->name);
    printf("%s:\n", g->name);
    printf("    .zero %d\n", g->ty->size);
  }
}

void emit_text(Function *prog) {
  printf("    .text\n");
  for (Function *f = prog; f; f = f->next) {
    cur_func = f;

    printf("    .globl %s\n", f->name);
    printf("%s:\n", f->name);
    prologue(f->lvars);

    for (Node *n = f->body; n; n = n->next)
      gen_stmt(n);

    epilogue();
  }
}

void codegen(Function *prog) {
  printf("    .intel_syntax noprefix\n");
  emit_data();
  emit_text(prog);
}
