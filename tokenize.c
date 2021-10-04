#include "occ.h"

Token *tokenize(char *input) {
  char *p = input;

  Token head = {};
  Token *cur = &head;;

  for (; *p;) {
    // skip space and '\n'
    if (*p == ' ' || *p == '\n') {
      p++;
      continue;
    }

    // skip single line comment
    if (strncmp(p, "//", 2) == 0) {
      for (; *p != '\n'; p++);
      p++;
      continue;
    }

    // skip multi-line comment
    if (strncmp(p, "/*", 2) == 0) {
      for (; strncmp(p, "*/", 2) != 0; p++);
      p += 2;
      continue;
    }

    // character (e.g. 'a')
    if (*p == '\'') {
      char *start = p;
      p++;

      if (!((65 <= *p && *p <= 90) || (97 <= *p && *p <= 122))) {
        fprintf(stderr, "invalid character: %s\n", p);
        exit(1);
      }
      p++;

      if (*p != '\'') {
        fprintf(stderr, "unclosed character: %s\n", start);
        exit(1);
      }
      p++;

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_CHAR;
      tok->loc = start;
      tok->len = 3;
      cur = cur->next = tok;
      continue;
    }

    // string (e.g. "Hello")
    if (*p == '"') {
      char *start = p;
      // Find closing '"'.
      for (p++; *p != '"'; p++);
      p++;

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_STR;
      tok->loc = start;
      tok->len = p - start;
      cur = cur->next = tok;
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
      if (equal(tok, "return") || equal(tok, "sizeof") ||
          equal(tok, "if") || equal(tok, "else") || equal(tok, "for") ||
          equal(tok, "void") || equal(tok, "int") || equal(tok, "char") ||
          equal(tok, "break") || equal(tok, "continue"))
        tok->kind = TK_KW;

      cur = cur->next = tok;
      continue;
    }

    // puctuator (two letters)
    if (strncmp(p, "++", 2) == 0 || strncmp(p, "--", 2) == 0 ||
        strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
        strncmp(p, "&&", 2) == 0 || strncmp(p, "||", 2) == 0) {
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
        *p == '<' || *p == '>' || *p == '[' || *p == ']') {
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
