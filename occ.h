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
  TK_IDENT,   // identifier
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
typedef struct Var Var;
struct Var {
  Var *next;
  char *name;
  int offset;
};

typedef enum {
  // statements
  ND_EXPR_STMT, // expression statement
  ND_RETURN,    // return statement

  ND_NUM,       // number
  ND_VAR,       // variable
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_ASSIGN,    // =
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok;

  // statement
  Node *next;
  Node *body;

  Node *lhs;
  Node *rhs;

  // ND_NUM
  int num;

  // ND_VAR
  Var *var;
};

typedef struct Function Function;
struct Function {
  Node *body;
  // local variables
  Var *vars;
};

Function *parse(Token *tok);

/* codegen.c */
void codegen(Function *func);

/* debug.c */
void debug_node(Node *node);
