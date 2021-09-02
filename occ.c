#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * tokenize
 */
typedef struct Token Token;
struct Token {
  char *loc;
  int len;

  Token *next;
};

// debugger
void debug_token(Token *tok) {
  printf("=== debug ===\n");
  printf("loc: %s\n", tok->loc);
  printf("len: %d\n", tok->len);
  printf("=============\n");
}

Token *tokenize(char *input) {
  char *p = input;

  Token *tok;

  if (isdigit(*p)) {
    char *start = p;

    p++;
    for (; isdigit(*p); p++);

    tok = calloc(1, sizeof(Token));
    tok->loc = start;
    tok->len = p - start;
  }

  return tok;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "only one argumet required\n");
    exit(1);
  }

  Token *tok = tokenize(argv[1]);

  printf("    .intel_syntax noprefix\n");
  printf("    .globl main\n");
  printf("main:\n");
  printf("    mov rax, %s\n", strndup(tok->loc, tok->len));
  printf("    ret\n");

  exit(0);
}
