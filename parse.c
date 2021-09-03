#include "occ.h"

_Bool equal(Token *tok, char *str) {
  return tok->len == strlen(str) && strncmp(tok->loc, str, tok->len) == 0;
}

Node *new_add_node(Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ADD;
  node->tok = tok;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_sub_node(Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_SUB;
  node->tok = tok;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *add(Token *tok, Token **rest);
Node *num(Token *tok, Token **rest);

// add = num ("+" num | "-" num)*
Node *add(Token *tok, Token **rest) {
  Node *node = num(tok, &tok);

  for (;;) {
    if (equal(tok, "+")) {
      node = new_add_node(node, num(tok->next, &tok), tok);
      continue;
    }

    if (equal(tok, "-")) {
      node = new_sub_node(node, num(tok->next, &tok), tok);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// num = number
Node *num(Token *tok, Token **rest) {
  if (tok->kind != TK_NUM) {
    fprintf(stderr, "number token is expected\n");
    exit(1);
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->tok = tok;
  node->num = atoi(strndup(tok->loc, tok->len));
  *rest = tok->next;
  return node;
}

Node *parse(Token *tok) {
  return add(tok, &tok);
}
