#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* tokenize.c */
typedef enum {
  TK_NUM, // number
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
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;
  Token *tok;

  // ND_NUM
  int num;
};

Node *parse(Token *tok);

/* codegen.c */
void codegen(Node *node);
