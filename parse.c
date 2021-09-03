#include "occ.h"

_Bool equal(Token *tok, char *str) {
  return tok->len == strlen(str) && strncmp(tok->loc, str, tok->len) == 0;
}

Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *add(Token *tok, Token **rest);
Node *mul(Token *tok, Token **rest);
Node *num(Token *tok, Token **rest);

// add = mul ("+" mul | "-" mul)*
Node *add(Token *tok, Token **rest) {
  Node *node = mul(tok, &tok);

  for (;;) {
    if (equal(tok, "+")) {
      node = new_binary_node(ND_ADD, node, mul(tok->next, &tok), tok);
      continue;
    }

    if (equal(tok, "-")) {
      node = new_binary_node(ND_SUB, node, mul(tok->next, &tok), tok);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// mul = num ("*" num | "/" num)*
Node *mul(Token *tok, Token **rest) {
  Node *node = num(tok, &tok);

  for (;;) {
    if (equal(tok, "*")) {
      node = new_binary_node(ND_MUL, node, num(tok->next, &tok), tok);
      continue;
    }

    if (equal(tok, "/")) {
      node = new_binary_node(ND_DIV, node, num(tok->next, &tok), tok);
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

// stmt = add ";"
Node *stmt(Token *tok, Token **rest) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT;
  node->tok = tok;
  node->lhs = add(tok, &tok);

  if (!equal(tok, ";")) {
    fprintf(stderr, "expected \";\"\n");
    exit(1);
  }

  *rest = tok->next;
  return node;
}

// stmt*
Node *parse(Token *tok) {
  Node head;
  Node *cur = &head;

  for (; tok->kind != TK_EOF;)
    cur = cur->next = stmt(tok, &tok);

  return head.next;
}
