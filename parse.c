#include "occ.h"

Node *parse(Token *tok) {
  Node head = {};
  Node *cur = &head;

  for (Token *t = tok; t->kind != TK_EOF; t = t->next) {
    switch (t->kind) {
      case TK_NUM: {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_NUM;
        node->tok = t;
        node->num = t->num;
        cur = cur->next = node;
        break;
      }
      default:
        fprintf(stderr, "unknown token kind: %d\n", t->kind);
        exit(1);
    }
  }

  return head.next;
}
