#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

/* tokenize.c */
typedef enum {
  TK_NUM,     // number
  TK_PUNCT,   // puctuators
  TK_KEYWORD, // keyword
  TK_EOF,     // end of token
} TokenKind;

typedef struct Token Token;
struct Token {
  Token *next;
  TokenKind kind;
  char *loc;
  int len;

  // TK_NUM
  int num;
};

Token *tokenize(char *input);

/* parse.c */
typedef enum {
  ND_STMT, // statement
  ND_NUM,  // number
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok;

  // ND_NUM
  int num;

  // ND_STMT
  Node *next;
  Node *body;

  Node *lhs;
  Node *rhs;
};

Node *parse(Token *tok);

/* codegen.c */
void codegen(Node *node);

/* debug.c */
void debug_node(Node *node);
