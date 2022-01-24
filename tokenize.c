#include "occ.h"

// Return keyword length if keyword,
// otherwise return zero.
int is_keyword(char *loc) {
  char *keywords[] = {";", "(", ")", "{", "}", "return"};
  for (int i = 0; i < sizeof(keywords) / sizeof(char*); i++) {
    char *kw = keywords[i];
    if (strncmp(loc, kw, strlen(kw)) == 0)
      return strlen(kw);
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

    // keyword
    if (is_keyword(p)) {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_KEYWORD;
      tok->loc = p;
      tok->len = is_keyword(p);
      cur = cur->next = tok;
      p += tok->len;
      continue;
    }

    // puctuator
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = 1;
      cur = cur->next = tok;
      p++;
      continue;
    }

    // identifier
    if (isalnum(*p)) {
      char *start = p;
      for (; isalnum(*p); p++);
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_IDENT;
      tok->loc = start;
      tok->len = p - start;
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
