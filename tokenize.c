#include "occ.h"

// debugger
void debug_tokens(Token *tok) {
  printf("=== debug ===\n");
  for (Token *t = tok; t; t = t->next) {
    if (t != tok)
      printf("---\n");

    printf("kind:\t%d\n", t->kind);
    printf("loc:\t\"%s\"\n", t->loc);
    printf("len:\t%d\n", t->len);
  }
  printf("=============\n");
}

Token *tokenize(char *input) {
  char *p = input;

  Token head = {};
  Token *cur = &head;;

  for (; *p != '\0';) {
    // skip spaces
    if (*p == ' ') {
      p++;
      continue;
    }

    // keyword / identifier
    if (strncmp(p, "return ", 7) == 0) {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_KW;
      tok->loc = p;
      tok->len = 6;
      cur = cur->next = tok;
      p = p + tok->len;
      continue;
    }

    // puctuator
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == ';') {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = 1;
      cur = cur->next = tok;
      p++;
      continue;
    }

    // number
    if (isdigit(*p)) {
      char *start = p;

      p++;
      for (; isdigit(*p); p++);

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_NUM;
      tok->loc = start;
      tok->len = p - start;
      cur = cur->next = tok;
      continue;
    }

    fprintf(stderr, "untokenized characters: %s\n", p);
    exit(1);
  }

  // Append EOF token.
  Token *eof = calloc(1, sizeof(Token));
  eof->kind = TK_EOF;
  eof->loc = p;
  eof->len = 0;
  cur->next = eof;

  return head.next;
}
