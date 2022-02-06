#include "occ.h"

// Currently processed function.
static Function *current_func;

// Registers where put arguments.
char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// Label index, used to identify labels.
int label_idx = 0;

// TODO: Process on parse.c
// Nested loop structure.
// (used by `break` and `continue` statements)
struct Loop {
  char *continue_label;
  char *break_label;
  struct Loop *outer;
};

struct Loop *current_loop = NULL;

void go_inner_loop(char *continue_label, char *break_label) {
  struct Loop *l = calloc(1, sizeof(struct Loop));
  l->continue_label = continue_label;
  l->break_label = break_label;
  l->outer = current_loop;
  current_loop = l;
}

void go_outer_loop() {
  current_loop = current_loop->outer;
}

void gen_expr(Node *node);

// Generate assembly code from statement nodes.
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
      int cnt = ++label_idx;
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
    case ND_FOR:
    case ND_DO:
    {
      int cnt = ++label_idx;
      char break_label[128];
      char continue_label[128];
      sprintf(break_label, ".L.for.%d.end", cnt);
      sprintf(continue_label, ".L.for.%d.increment", cnt);
      go_inner_loop(continue_label, break_label);

      if (node->init) {
        gen_expr(node->init);
        printf("  pop rax\n");
      }
      printf(".L.for.%d.start:\n", cnt);
      if (node->cond) {
        gen_expr(node->cond);
        printf("  pop rax\n");
        printf("  test rax, rax\n");
        printf("  jz .L.for.%d.end\n", cnt);
      }
      gen_stmt(node->body);
      printf("%s:\n", continue_label);
      if (node->inc) {
        gen_expr(node->inc);
        printf("  pop rax\n");
      }
      printf("  jmp .L.for.%d.start\n", cnt);
      printf("%s:\n", break_label);

      go_outer_loop();
      return;
    }
    case ND_BREAK:
       if (!current_loop || !current_loop->break_label) {
         fprintf(stderr, "break out of loop\n");
         exit(1);
       }
       printf("  jmp %s\n", current_loop->break_label);
       return;
    case ND_CONTINUE:
       if (!current_loop || !current_loop->continue_label) {
         fprintf(stderr, "continue out of loop\n");
         exit(1);
       }
       printf("  jmp %s\n", current_loop->continue_label);
       return;
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

// Push an address of a lvalue on stack top.
void gen_addr(Node *node) {
  if (!node->var) {
    fprintf(stderr, "not a lvalue\n");
    exit(1);
  }

  printf("  lea rax, [rbp - %d]\n", node->var->offset);
  printf("  push rax\n");
}

// Generate assembly code from expression nodes.
// (Each generated code intends to leave a result of an expression on the stack top.)
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
    case ND_LT: {
      int cnt = ++label_idx;
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  cmp rax, rdi\n");
      printf("  jl .L.lt.%d.true\n", cnt);
      printf("  jmp .L.lt.%d.false\n", cnt);
      printf(".L.lt.%d.true:\n", cnt);
      printf("  push 1\n");
      printf("  jmp .L.lt.%d.end\n", cnt);
      printf(".L.lt.%d.false:\n", cnt);
      printf("  push 0\n");
      printf(".L.lt.%d.end:\n", cnt);
      return;
    }
    case ND_LTE: {
      int cnt = ++label_idx;
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  cmp rax, rdi\n");
      printf("  jle .L.lte.%d.true\n", cnt);
      printf("  jmp .L.lte.%d.false\n", cnt);
      printf(".L.lte.%d.true:\n", cnt);
      printf("  push 1\n");
      printf("  jmp .L.lte.%d.end\n", cnt);
      printf(".L.lte.%d.false:\n", cnt);
      printf("  push 0\n");
      printf(".L.lte.%d.end:\n", cnt);
      return;
    }
    case ND_EQ: {
      int cnt = ++label_idx;
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  cmp rax, rdi\n");
      printf("  je .L.eq.%d.true\n", cnt);
      printf("  jmp .L.eq.%d.false\n", cnt);
      printf(".L.eq.%d.true:\n", cnt);
      printf("  push 1\n");
      printf("  jmp .L.eq.%d.end\n", cnt);
      printf(".L.eq.%d.false:\n", cnt);
      printf("  push 0\n");
      printf(".L.eq.%d.end:\n", cnt);
      return;
    }
    case ND_NEQ: {
      int cnt = ++label_idx;
      gen_expr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  cmp rax, rdi\n");
      printf("  jne .L.eq.%d.true\n", cnt);
      printf("  jmp .L.eq.%d.false\n", cnt);
      printf(".L.eq.%d.true:\n", cnt);
      printf("  push 1\n");
      printf("  jmp .L.eq.%d.end\n", cnt);
      printf(".L.eq.%d.false:\n", cnt);
      printf("  push 0\n");
      printf(".L.eq.%d.end:\n", cnt);
      return;
    }
    case ND_ASSIGN:
      gen_addr(node->lhs);
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_FUNCALL:
    {
      int i = 0;
      for (Node *arg = node->args; arg; arg = arg->next) {
        gen_expr(arg);
        printf("  pop %s\n", arg_regs[i]);
        ++i;
      }
      printf("  call %s\n", node->func_name);
      printf("  push rax\n");
      return;
    }
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

// Assign offset to local variables and stack area for them.
void assign_lvar_offset(Function *func) {
  int offset = 0;

  // arguments
  for (Var *arg = func->args; arg; arg = arg->next) {
    offset += 8;
    arg->offset = offset;
  }

  // local variables
  for (Var *v = func->vars; v; v = v->next) {
    offset += 8;
    v->offset = offset;
  }

  printf("  sub rsp, %d\n", offset);
}

// Assign arguments to memory according to their offsets.
void assign_args_to_mem(Var *args) {
  int i = 0;
  for (Var *arg = args; arg; arg = arg->next) {
    printf("  lea rax, [rbp - %d]\n", arg->offset);
    printf("  mov [rax], %s\n", arg_regs[i]);
    ++i;
  }
}

void codegen(Function *func) {
  printf("  .intel_syntax noprefix\n");
  printf("  .text\n");
  printf("  .globl main\n");

  for (current_func = func; current_func; current_func = current_func->next) {
    printf("%s:\n", current_func->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    assign_lvar_offset(current_func);
    assign_args_to_mem(current_func->args);

    for (Node *node = current_func->body; node; node = node->next)
      gen_stmt(node);

    printf(".L.%s.end:\n", current_func->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}
