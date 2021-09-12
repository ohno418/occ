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

  for (; *p;) {
    // skip spaces
    if (*p == ' ') {
      p++;
      continue;
    }

    // keyword / identifier
    if (isalpha(*p)) {
      char *start = p;
      for (; isalpha(*p) || isdigit(*p) || *p == '_'; p++);

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_IDENT;
      tok->loc = start;
      tok->len = p - start;

      // keyword
      if (equal(tok, "return") || equal(tok, "int") || equal(tok, "sizeof") ||
          equal(tok, "if") || equal(tok, "else") || equal(tok, "for"))
        tok->kind = TK_KW;

      cur = cur->next = tok;
      continue;
    }

    // puctuator (two letters)
    if (strncmp(p, "++", 2) == 0 || strncmp(p, "--", 2) == 0 ||
        strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0) {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = 2;
      p += 2;
      cur = cur->next = tok;
      continue;
    }

    // puctuator
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '{' || *p == '}' ||
        *p == ';' || *p == '=' || *p == '&' || *p == ',' ||
        *p == '<' || *p == '>') {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = 1;
      p++;
      cur = cur->next = tok;
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
