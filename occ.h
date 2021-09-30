#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct Type Type;

/*
 * tokenize.c
 */
typedef enum TokenKind {
  TK_NUM,   // number
  TK_CHAR,  // char
  TK_STR,   // string
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

  // global variable
  bool is_global;
  // function argument
  bool is_arg;
};

typedef enum NodeKind {
  // statements
  //   - lhs:  body
  //   - next: next statement
  ND_STMT,
  ND_RETURN,  // return
  ND_IF,      // if
  ND_FOR,     // for
  ND_BLOCK,   // compound statement (block)

  ND_NUM,     // number
  ND_CHAR,    // character
  ND_STR,     // string
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_EQ,      // ==
  ND_NEQ,     // !=
  ND_LT,      // <
  ND_LTE,     // <=
  ND_AND,     // &&
  ND_OR,      // ||
  ND_REF,     // address operator &
  ND_DEREF,   // dereference operator *
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
  // ND_CHAR
  int num;

  // ND_STR
  char *str;

  // ND_VAR
  Var *var;

  // ND_FUNCALL
  char *func_name;
  Node *args;

  // ND_IF:  cond, then, els
  // ND_FOR: init, cond, inc, then
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // ND_BLOCK
  Node *body;
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

// list of global variables
extern Var *gvars;
// list of string literals
extern Node *strs;

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
  TY_CHAR,
  TY_INT,
  TY_PTR,
  TY_ARRAY,
} TypeKind;

struct Type {
  TypeKind kind;
  int size;

  // TY_PTR
  // TY_ARRAY
  Type *base;

  // variable (or function) name holder
  char *name;
};

Type *ty_char();
Type *ty_int();
Type *ty_ptr(Type *base);
Type *ty_array(Type *base, int num);
Type *type_of(Node *node);
