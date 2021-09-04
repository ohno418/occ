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
  TK_IDENT, // identifier
  TK_KW,    // keyword
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
 * parse.c
 */
typedef enum NodeKind {
  // statements
  //   - lhs:  body
  //   - next: next statement
  ND_STMT,
  ND_RETURN, // return

  ND_NUM,    // number
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_VAR,    // variable
  ND_ASSIGN, // assign
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok; // corresponding token (for debugging)

  // statement
  Node *next;

  Node *lhs;  // left-hand
  Node *rhs;  // right-hand

  int num;    // ND_NUM

  // ND_VAR
  char *name;
  int offset;
};

_Bool equal(Token *tok, char *str);

Node *parse(Token *tok);

/**
 * codegen.c
 */
void codegen(Node *node);
