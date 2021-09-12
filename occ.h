#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct Type Type;

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
  Type *ty;
  char *name;
  int offset;
  Var *next;

  // function argument
  bool is_arg;
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
  ND_REF,     // address operator &
  ND_DEREF,   // dereference operator *
  ND_VAR,     // variable
  ND_ASSIGN,  // assign
  ND_IF,      // if
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
  Node *args;

  // ND_IF
  Node *cond;
  Node *then;
  Node *els;
};

typedef struct Function Function;
struct Function {
  Type *ty;
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

/*
 * type.c
 */
typedef enum TypeKind {
  TY_INT,
  TY_PTR,
} TypeKind;

struct Type {
  TypeKind kind;
  int size;

  // TY_PTR
  Type *base;

  // variable (or function) name holder
  char *name;
};

Type *ty_int();
Type *ty_ptr();
