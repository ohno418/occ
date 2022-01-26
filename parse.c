#include "occ.h"

// local variables
Var *lvars = NULL;

Var *register_lvar(char *name) {
  // Does the name already exist?
  for (Var *v = lvars; v; v = v->next) {
    if (strlen(name) == strlen(v->name) &&
        strncmp(name, v->name, strlen(name)) == 0) {
      return v;
    }
  }

  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = lvars;
  lvars = var;
  return var;
}

bool equal(Token *tok, char *str) {
  if (!tok->loc) {
    return false;
  }

  return strncmp(tok->loc, str, strlen(str)) == 0;
}

void consume(Token *tok, Token **rest, char *str) {
  if (equal(tok, str)) {
    *rest = tok->next;
  } else {
    fprintf(stderr, "expected \"%s\": %s\n", str, tok->loc);
    exit(1);
  }
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->tok = tok;
  return node;
}

Node *stmt(Token *tok, Token **rest);
Node *expr(Token *tok, Token **rest);
Node *assign(Token *tok, Token **rest);
Node *add(Token *tok, Token **rest);
Node *mul(Token *tok, Token **rest);
Node *primary(Token *tok, Token **rest);

// stmt = "return" expr ";"
//      | expr ";"
Node *stmt(Token *tok, Token **rest) {
  if (equal(tok, "return")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->tok = tok;
    node->body = expr(tok->next, &tok);
    consume(tok, rest, ";");
    return node;
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_STMT;
  node->tok = tok;
  node->body = expr(tok, &tok);
  consume(tok, rest, ";");
  return node;
}

// expr = assign
Node *expr(Token *tok, Token **rest) {
  return assign(tok, rest);
}

// assign = add ("=" assign)*
Node *assign(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = add(tok, &tok);

  for (; equal(tok, "=");) {
    Node *rhs = assign(tok->next, &tok);
    node = new_binary(ND_ASSIGN, node, rhs, start);
    continue;
  }

  *rest = tok;
  return node;
}

// add = mul (("+" | "-") mul)*
Node *add(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = mul(tok, &tok);

  for (;;) {
    if (strncmp(tok->loc, "+", tok->len) == 0) {
      Node *rhs = mul(tok->next, &tok);
      node = new_binary(ND_ADD, node, rhs, start);
      continue;
    }

    if (strncmp(tok->loc, "-", tok->len) == 0) {
      Node *rhs = mul(tok->next, &tok);
      node = new_binary(ND_SUB, node, rhs, start);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// mul = primary (("*" | "/") primary)*
Node *mul(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = primary(tok, &tok);

  for (; tok->kind == TK_PUNCT;) {
    if (equal(tok, "*")) {
      Node *rhs = primary(tok->next, &tok);
      node = new_binary(ND_MUL, node, rhs, start);
      continue;
    }

    if (equal(tok, "/")) {
      Node *rhs = primary(tok->next, &tok);
      node = new_binary(ND_DIV, node, rhs, start);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// primary = identifier
//         | number
Node *primary(Token *tok, Token **rest) {
  // identifier
  if (tok->kind == TK_IDENT) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = tok;
    node->var = register_lvar(strndup(tok->loc, tok->len));
    *rest = tok->next;
    return node;
  }

  // number
  if (tok->kind == TK_NUM) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->tok = tok;
    node->num = tok->num;
    *rest = tok->next;
    return node;
  }

  fprintf(stderr, "unknown primary expression: %s\n", tok->loc);
  exit(1);
}

// function = "main" "(" ")" "{" stmt* "}"
Function *function(Token *tok, Token **rest) {
  if (!(equal(tok, "main") &&
        equal(tok->next, "(") && equal(tok->next->next, ")") &&
        equal(tok->next->next->next, "{"))) {
    fprintf(stderr, "function format is wrong: %s\n", tok->loc);
    exit(1);
  }
  tok = tok->next->next->next->next;

  Function *func = calloc(1, sizeof(Function));
  lvars = NULL;

  // AST of body
  Node head;
  Node *cur = &head;
  for (; !equal(tok, "}");)
    cur = cur->next = stmt(tok, &tok);
  consume(tok, rest, "}");

  func->body = head.next;
  func->vars = lvars;
  return func;
}

Function *parse(Token *tok) {
  Function *func = function(tok, &tok);

  if (tok->kind != TK_EOF) {
    fprintf(stderr, "expected TK_EOF token: %d\n", tok->kind);
    exit(1);
  }

  return func;
}
