#include "occ.h"

bool equal(Token *tok, char *str) {
  return tok->len == strlen(str) && strncmp(tok->loc, str, tok->len) == 0;
}

Node vars;
Node *cur_var = &vars;
int var_offset = 0;

Node *find_var(char *name) {
  for (Node *v = vars.next; v; v = v->next)
    if (strlen(v->name) == strlen(name) &&
        strncmp(v->name, name, strlen(name)) == 0)
      return v;

  return NULL;
}

Node *new_num_node(int num, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->tok = tok;
  node->num = num;
  return node;
}

Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *expr(Token *tok, Token **rest);
Node *assign(Token *tok, Token **rest);
Node *add(Token *tok, Token **rest);
Node *mul(Token *tok, Token **rest);
Node *postfix(Token *tok, Token **rest);
Node *primary(Token *tok, Token **rest);

// expr = assign
Node *expr(Token *tok, Token **rest) {
  return assign(tok, rest);
}

// assign = ident "=" add
//        | add
Node *assign(Token *tok, Token **rest) {
  if (tok->kind == TK_IDENT && equal(tok->next, "=")) {
    Node *lhs = calloc(1, sizeof(Node));
    lhs->kind = ND_VAR;
    lhs->tok = tok;
    char *var_name = strndup(tok->loc, tok->len);
    Node *var_node = find_var(var_name);
    if (!var_node) {
      var_offset += 8;
      cur_var = cur_var->next = lhs;
    }
    lhs->name = var_name;
    lhs->offset = var_offset;
    Node *node = new_binary_node(ND_ASSIGN,
        lhs, add(tok->next->next, &tok), tok);

    *rest = tok;
    return node;
  }

  return add(tok, rest);
}

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

// mul = postfix ("*" postfix | "/" postfix)*
Node *mul(Token *tok, Token **rest) {
  Node *node = postfix(tok, &tok);

  for (;;) {
    if (equal(tok, "*")) {
      node = new_binary_node(ND_MUL, node, postfix(tok->next, &tok), tok);
      continue;
    }

    if (equal(tok, "/")) {
      node = new_binary_node(ND_DIV, node, postfix(tok->next, &tok), tok);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// postfix = primary ("++" | "--")?
Node *postfix(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = primary(tok, &tok);

  if (equal(tok, "++")) {
    node = new_binary_node(
        ND_ASSIGN,
        node,
        new_binary_node(
            ND_ADD,
            node,
            new_num_node(1, start),
            start
        ),
        start
    );
    tok = tok->next;
  }

  if (equal(tok, "--")) {
    node = new_binary_node(
        ND_ASSIGN,
        node,
        new_binary_node(
            ND_SUB,
            node,
            new_num_node(1, start),
            start
        ),
        start
    );
    tok = tok->next;
  }

  *rest = tok;
  return node;
}

// primary = number
//         | ident
Node *primary(Token *tok, Token **rest) {
  // number
  if (tok->kind == TK_NUM) {
    Node *node = new_num_node(atoi(strndup(tok->loc, tok->len)), tok);
    *rest = tok->next;
    return node;
  }

  // ident (variable)
  if (tok->kind == TK_IDENT) {
    char *var_name = strndup(tok->loc, tok->len);
    Node *var_node = find_var(var_name);
    if (!var_node) {
      fprintf(stderr, "unknown variable: %s\n", var_name);
      exit(1);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = tok;
    node->name = var_node->name;
    node->offset = var_node->offset;
    *rest = tok->next;
    return node;
  }

  fprintf(stderr, "unknown primary: %s\n", tok->loc);
  exit(1);
}

// stmt = "return" expr ";"
//      | expr ";"
Node *stmt(Token *tok, Token **rest) {
  NodeKind kind = ND_STMT;
  if (equal(tok, "return")) {
    kind = ND_RETURN;
    tok = tok->next;
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  node->lhs = expr(tok, &tok);

  if (!equal(tok, ";")) {
    fprintf(stderr, "expected \";\"\n");
    exit(1);
  }

  *rest = tok->next;
  return node;
}

// "main" "(" ")" "{" stmt* "}"
Node *parse(Token *tok) {
  if (!(equal(tok, "main") && equal(tok->next, "(") &&
      equal(tok->next->next, ")") && equal(tok->next->next->next, "{"))) {
    fprintf(stderr, "main function required: %s\n", tok->loc);
    exit(1);
  }
  tok = tok->next->next->next->next;

  Node head;
  Node *cur = &head;

  for (; !equal(tok, "}");)
    cur = cur->next = stmt(tok, &tok);

  if (tok->next->kind != TK_EOF) {
    fprintf(stderr, "extra token\n");
    exit(1);
  }

  return head.next;
}
