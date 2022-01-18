#include "occ.h"

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->tok = tok;
  return node;
}

Node *expr(Token *tok, Token **rest);
Node *num(Token *tok, Token **rest);

// num (+ expr)?
Node *expr(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = num(tok, &tok);

  if (tok->kind == TK_ADD) {
    Node *rhs = expr(tok->next, &tok);
    node = new_binary(ND_ADD, node, rhs, start);
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

Node *parse(Token *tok) {
  return expr(tok, &tok);
}
