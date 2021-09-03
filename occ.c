#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * tokenize
 */
typedef enum TokenKind {
  TK_NUM,  // number
  TK_PUNCT, // punctuator
  TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
  Token *next;
  TokenKind kind;
  char *loc;
  int len;
};

// debugger
void debug_tokens(Token *tok) {
  printf("=== debug ===\n");
  for (Token *t = tok; t; t = t->next) {
    if (t != tok)
      printf("---\n");

    printf("kind:\t%d\n", t->kind);
    printf("loc:\t\"%s\"\n", t->loc);
    printf("len:\t%d\n", t->len);
  }
  printf("=============\n");
}

Token *tokenize(char *input) {
  char *p = input;

  Token head = {};
  Token *cur = &head;;

  for (; *p != '\0';) {
    // skip spaces
    if (*p == ' ') {
      p++;
      continue;
    }

    // number
    if (isdigit(*p)) {
      char *start = p;

      p++;
      for (; isdigit(*p); p++);

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_NUM;
      tok->loc = start;
      tok->len = p - start;
      cur = cur->next = tok;
      continue;
    }

    // +
    if (*p == '+') {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = 1;
      cur = cur->next = tok;
      p++;
      continue;
    }

    fprintf(stderr, "untokenized characters: %s\n", p);
    exit(1);
  }

  // Append EOF token.
  Token *eof = calloc(1, sizeof(Token));
  eof->kind = TK_EOF;
  eof->loc = p;
  eof->len = 0;
  cur->next = eof;

  return head.next;
}

/**
 * parse
 */
typedef enum NodeKind {
  ND_NUM, // number
  ND_ADD, // +
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok; // corresponding token (for debugging)

  int num;    // ND_NUM

  Node *lhs;  // left-hand
  Node *rhs;  // right-hand
};

_Bool equal(Token *tok, char *str) {
  return tok->len == strlen(str) && strncmp(tok->loc, str, tok->len) == 0;
}

Node *new_add_node(Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ADD;
  node->tok = tok;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *add(Token *tok, Token **rest);
Node *num(Token *tok, Token **rest);

// add = num ("+" add)*
Node *add(Token *tok, Token **rest) {
  Node *node = num(tok, &tok);

  if (equal(tok, "+"))
    node = new_add_node(node, add(tok->next, &tok), tok);

  *rest = tok;
  return node;
}

// num = number
Node *num(Token *tok, Token **rest) {
  if (tok->kind != TK_NUM) {
    fprintf(stderr, "number token is expected\n");
    exit(1);
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->tok = tok;
  node->num = atoi(strndup(tok->loc, tok->len));
  *rest = tok->next;
  return node;
}

Node *parse(Token *tok) {
  return add(tok, &tok);
}

/**
 * codegen
 */
// Push the result to the stack.
void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("    push %d\n", node->num);
    break;
  case ND_ADD:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    printf("    pop rax\n");
    printf("    pop rdi\n");
    printf("    add rax, rdi\n");
    printf("    push rax\n");
    break;
  default:
    fprintf(stderr, "unknown kind of node: %d\n", node->kind);
    exit(1);
  }
}

void codegen(Node *node) {
  printf("    .intel_syntax noprefix\n");
  printf("    .globl main\n");
  printf("main:\n");

  gen_expr(node);

  printf("    pop rax\n");
  printf("    ret\n");
}

/**
 * main
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "only one argumet required\n");
    exit(1);
  }

  Token *tok = tokenize(argv[1]);
  Node *node = parse(tok);
  codegen(node);

  exit(0);
}
