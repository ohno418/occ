#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* tokenize.c */
typedef enum {
  TK_NUM, // number
  TK_ADD, // +
  TK_EOF, // end of token
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  char *loc;
  int len;

  // TK_NUM
  int num;
};

Token *tokenize(char *input);

/* parse.c */
typedef enum {
  ND_NUM, // number
  ND_ADD, // add
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok;

  // ND_NUM
  int num;

  Node *lhs;
  Node *rhs;
};

Node *parse(Token *tok);

/* codegen.c */
void codegen(Node *node);
