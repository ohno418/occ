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
typedef struct Type Type;
typedef struct Var Var;
struct Var {
  Var *next;
  Type *ty;
  char *name;
  int offset;
};

typedef enum {
  // statements
  ND_EXPR_STMT, // expression statement
  ND_BLOCK,     // compound statement
  ND_IF,        // if statement
                // (with `cond` and `els` fields)
  /*
  ND_FOR,       // for statement
                // (with `init`, `cond`, `inc` fields)
  */
  ND_RETURN,    // return statement
  ND_NULL_STMT, // null statement

  ND_NUM,       // number
  ND_VAR,       // variable
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_LT,        // <
  ND_LTE,       // <=
  ND_EQ,        // ==
  ND_NEQ,       // !=
  ND_ASSIGN,    // =
  ND_FUNCALL,   // function call
  ND_COMMA,     // comma expression
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Token *tok;

  // statement
  Node *next;
  Node *body;

  // ND_IF
  Node *cond;
  Node *els;

  // ND_FOR
  // Node *init;
  // Node *inc;

  Node *lhs;
  Node *rhs;

  // ND_NUM
  int num;

  // ND_VAR
  Var *var;

  // ND_FUNCALL
  char *func_name;
};

typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  Type *ty;
  Node *body;
  // local variables
  Var *vars;
};

Function *parse(Token *tok);

/* codegen.c */
void codegen(Function *func);

/* type.c */
typedef enum {
  TY_INT, // int
} TypeKind;

struct Type {
  TypeKind kind;
  int size;
};

Type *ty_int();

/* debug.c */
void debug_node(Node *node);
