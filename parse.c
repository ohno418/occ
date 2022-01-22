#include "occ.h"

bool equal(Token *tok, char *str) {
  if (!tok->loc) {
    return false;
  }

  return strncmp(tok->loc, str, strlen(str)) == 0;
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

// stmt = expr ";"
Node *stmt(Token *tok, Token **rest) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT;
  node->tok = tok;
  node->body = expr(tok, &tok);

  if (!equal(tok, ";")) {
    fprintf(stderr, "expected \";\": %s\n", tok->loc);
    exit(1);
  }

  *rest = tok->next;
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

Node *parse(Token *tok) {
  Node head;
  Node *cur = &head;

  for (; tok->kind != TK_EOF;)
    cur = cur->next = stmt(tok, &tok);

  return head.next;
}
