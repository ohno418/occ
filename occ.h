#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/*
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

/*
 * parse.c
 */
typedef struct Var Var;
struct Var {
  char *name;
  int offset;
  Var *next;
};

typedef enum NodeKind {
  // statements
  //   - lhs:  body
  //   - next: next statement
  ND_STMT,
  ND_RETURN,  // return

  ND_NUM,     // number
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_VAR,     // variable
  ND_ASSIGN,  // assign
  ND_FUNCALL, // function call
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok; // corresponding token (for debugging)

  // statement
  Node *next;

  Node *lhs;  // left-hand
  Node *rhs;  // right-hand

  // ND_NUM
  int num;

  // ND_VAR
  Var *var;

  // ND_FUNCALL
  char *func_name;
};

typedef struct Function Function;
struct Function {
  char *name;
  Node *body;
  Var *lvars;
  Function *next;
};

bool equal(Token *tok, char *str);

Function *parse(Token *tok);

// debugger
void debug_node(Node *node);

/*
 * codegen.c
 */
void codegen(Function *prog);
