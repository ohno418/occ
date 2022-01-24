#include "occ.h"

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
Node *mul(Token *tok, Token **rest);
Node *num(Token *tok, Token **rest);

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
  node->kind = ND_STMT;
  node->tok = tok;
  node->body = expr(tok, &tok);
  consume(tok, rest, ";");
  return node;
}

// expr = mul (("+" | "-") mul)*
Node *expr(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = mul(tok, &tok);

  for (; tok->kind == TK_PUNCT;) {
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

    assert(0);
  }

  *rest = tok;
  return node;
}

// mul = num (("*" | "/") num)*
Node *mul(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = num(tok, &tok);

  for (; tok->kind == TK_PUNCT;) {
    if (strncmp(tok->loc, "*", tok->len) == 0) {
      Node *rhs = num(tok->next, &tok);
      node = new_binary(ND_MUL, node, rhs, start);
      continue;
    }

    if (strncmp(tok->loc, "/", tok->len) == 0) {
      Node *rhs = num(tok->next, &tok);
      node = new_binary(ND_DIV, node, rhs, start);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

Node *num(Token *tok, Token **rest) {
  if (tok->kind != TK_NUM) {
    fprintf(stderr, "expected a number: %s\n", tok->loc);
    exit(1);
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->tok = tok;
  node->num = tok->num;
  *rest = tok->next;
  return node;
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

  Node head;
  Node *cur = &head;

  for (; !equal(tok, "}");)
    cur = cur->next = stmt(tok, &tok);
  tok = tok->next;

  if (tok->kind != TK_EOF) {
    fprintf(stderr, "expected TK_EOF token: %d\n", tok->kind);
    exit(1);
  }

  Function *func = calloc(1, sizeof(Function));
  func->body = head.next;
  return func;
}

Function *parse(Token *tok) {
  return function(tok, &tok);
}
