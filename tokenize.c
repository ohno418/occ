#include "occ.h"

// Return keyword length if keyword,
// otherwise return zero.
int is_keyword(Token *tok) {
  char *keywords[] = {"return", "if", "sizeof", "int"};
  for (int i = 0; i < sizeof(keywords) / sizeof(char*); i++) {
    char *kw = keywords[i];
    if (tok->len == strlen(kw) &&
        strncmp(tok->loc, kw, strlen(kw)) == 0) {
      return strlen(kw);
    }
  }
  return 0;
}

Token *tokenize(char *input) {
  Token head = {};
  Token *cur = &head;

  char *p = input;
  for (; *p;) {
    // skip spaces
    if (isspace(*p)) {
      p++;
      continue;
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

    // puctuator
    if (ispunct(*p)) {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = 1;
      cur = cur->next = tok;
      p++;
      continue;
    }

    // identifier or keyword
    if (isalpha(*p)) {
      char *start = p;
      for (; isalnum(*p); p++);
      Token *tok = calloc(1, sizeof(Token));
      tok->loc = start;
      tok->len = p - start;
      cur = cur->next = tok;
      if (is_keyword(tok)) {
        tok->kind = TK_KEYWORD;
      } else {
        tok->kind = TK_IDENT;
      }
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
