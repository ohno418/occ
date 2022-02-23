#include "occ.h"

// Return keyword length if keyword,
// otherwise return zero.
int is_keyword(Token *tok) {
  char *keywords[] = {"return", "if", "else", "for", "while", "do", "break", "continue",
                      "sizeof", "char", "int"};
  for (int i = 0; i < sizeof(keywords) / sizeof(char*); i++) {
    char *kw = keywords[i];
    if (tok->len == strlen(kw) &&
        strncmp(tok->loc, kw, strlen(kw)) == 0) {
      return strlen(kw);
    }
  }
  return 0;
}

// Read a punctuator token from p and return its length.
int read_punct(char *p) {
  char *kw[] = {"++", "--", "<=", ">=", "==", "!="};
  for (int i = 0; i < sizeof(kw) / sizeof(char*); i++)
    if (strncmp(p, kw[i], strlen(kw[i])) == 0)
      return strlen(kw[i]);

  if (ispunct(*p))
    return 1;

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

    // char literal
    if (*p == '\'') {
      char *start = p;
      p++;

      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_NUM;
      tok->loc = start;
      tok->num = *p;
      p++;

      if (*p != '\'') {
        fprintf(stderr, "expected \"'\": %s", p);
        exit(1);
      }
      p++;

      tok->len = p - start;
      cur = cur->next = tok;
      continue;
    }

    // identifier or keyword
    if (isalpha(*p)) {
      char *start = p;
      for (; isalnum(*p) || *p == '_'; p++);
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

    // puctuator
    int punct_len = read_punct(p);
    if (punct_len) {
      Token *tok = calloc(1, sizeof(Token));
      tok->kind = TK_PUNCT;
      tok->loc = p;
      tok->len = punct_len;
      cur = cur->next = tok;
      p += punct_len;
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
