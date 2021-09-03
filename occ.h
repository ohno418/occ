#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * tokenize.c
 */
typedef enum TokenKind {
  TK_NUM,   // number
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

Token *tokenize(char *input);

/**
 * parse
 */
typedef enum NodeKind {
  ND_NUM, // number
  ND_ADD, // +
  ND_SUB, // -
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok; // corresponding token (for debugging)

  int num;    // ND_NUM

  Node *lhs;  // left-hand
  Node *rhs;  // right-hand
};

Node *parse(Token *tok);

/**
 * codegen
 */
void codegen(Node *node);
