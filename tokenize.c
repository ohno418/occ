#include "occ.h"

Token *tokenize(char *input) {
  Token head = {};
  Token *cur = &head;

  char *p = input;
  for (; *p;) {
    // skip spaces
    if (isspace(*p)) {
      p++;
    }

    // number
    if (isdigit(*p)) {
      char *start = p;
      for (; isdigit(*p); p++);

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_NUM;
      tok->loc = start;
      tok->len = p - start;
      tok->num = atoi(strndup(tok->loc, tok->len));
      cur = cur->next = tok;

      continue;
    }

    fprintf(stderr, "unknown input: %s\n", p);
    exit(1);
  }

  Token *eof = calloc(1, sizeof(Token));
  eof->kind = TK_EOF;
  cur->next = eof;

  return head.next;
}
