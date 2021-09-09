#include "occ.h"

// debugger
void debug_node(Node *node) {
  printf("=== debug ===\n");
  printf("kind:\t%d\n", node->kind);
  printf("loc:\t%s\n", node->tok->loc);
  printf("=========-===\n");
}

bool equal(Token *tok, char *str) {
  return tok->len == strlen(str) && strncmp(tok->loc, str, tok->len) == 0;
}

// List of local vars of current function.
Var *lvars;

void register_lvar(Var *var) {
  var->next = lvars;
  lvars = var;
}

Var *new_lvar(Type *ty, char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->ty = ty;
  var->name = name;
  register_lvar(var);
  return var;
}

Var *find_lvar(char *name) {
  for (Var *v = lvars; v; v = v->next)
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

bool is_typename(Token *tok) {
  if (equal(tok, "int"))
    return ty_int();

  return NULL;
}

// type_with_name = type-name "*"? var-name
Type *type_with_name(Token *tok, Token **rest) {
  Type *ty;
  if (equal(tok, "int")) {
    ty = ty_int();
  } else {
    fprintf(stderr, "not type name: %s\n", tok->loc);
    exit(1);
  }
  tok = tok->next;

  if (equal(tok, "*")) {
    ty = ty_ptr(ty);
    tok = tok->next;
  }

  ty->name = strndup(tok->loc, tok->len);

  *rest = tok->next;
  return ty;
}

Node *stmt(Token *tok, Token **rest);
Node *expr(Token *tok, Token **rest);
Node *assign(Token *tok, Token **rest);
Node *add(Token *tok, Token **rest);
Node *mul(Token *tok, Token **rest);
Node *postfix(Token *tok, Token **rest);
Node *primary(Token *tok, Token **rest);

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

// expr = assign
Node *expr(Token *tok, Token **rest) {
  return assign(tok, rest);
}

// assign = add ("=" assign)*
Node *assign(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = add(tok, &tok);

  for (; equal(tok, "=");) {
    node = new_binary_node(ND_ASSIGN,
        node, assign(tok->next, &tok), start);
  }

  *rest = tok;
  return node;
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
//         | ident "(" ")"
//         | "sizeof" "(" ident ")"
//         | "&" primary
//         | "*" primary
//         | type-name ident
//         | ident
Node *primary(Token *tok, Token **rest) {
  // number
  if (tok->kind == TK_NUM) {
    Node *node = new_num_node(atoi(strndup(tok->loc, tok->len)), tok);
    *rest = tok->next;
    return node;
  }

  // function call
  if (tok->kind == TK_IDENT &&
      equal(tok->next, "(") && equal(tok->next->next, ")")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCALL;
    node->tok = tok;
    node->func_name = strndup(tok->loc, tok->len);;
    *rest = tok->next->next->next;
    return node;
  }

  // sizeof
  if (equal(tok, "sizeof")) {
    if (!equal(tok->next, "(") || !equal(tok->next->next->next, ")")) {
      fprintf(stderr, "\"()\" is required after a sizeof operator\n", tok->loc);
      exit(1);
    }

    Token *var_tok = tok->next->next;
    char *var_name = strndup(var_tok->loc, var_tok->len);
    Var *lvar = find_lvar(var_name);
    if (!lvar) {
      fprintf(stderr, "unknown variable \"%s\"\n", var_name);
      exit(1);
    }

    Node *node = new_num_node(lvar->ty->size, tok);
    *rest = var_tok->next->next;
    return node;
  }

  // reference
  if (equal(tok, "&")) {
    Token *start = tok;
    Node *lhs = primary(tok->next, &tok);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_REF;
    node->tok = start;
    node->lhs = lhs;

    *rest = tok;
    return node;
  }

  // dereference
  if (equal(tok, "*")) {
    Token *start = tok;
    Node *lhs = primary(tok->next, &tok);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEREF;
    node->tok = start;
    node->lhs = lhs;

    *rest = tok;
    return node;
  }

  // new variable
  if (is_typename(tok)) {
    Token *start = tok;

    Type *ty = type_with_name(tok, &tok);
    Var *lvar = find_lvar(ty->name);
    if (lvar) {
      fprintf(stderr, "variable \"%s\" is already declared\n", ty->name);
      exit(1);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = start;
    node->var = new_lvar(ty, ty->name);

    *rest = tok;
    return node;
  }

  // existing variable
  if (tok->kind == TK_IDENT) {
    char *var_name = strndup(tok->loc, tok->len);
    Var *lvar = find_lvar(var_name);
    if (!lvar) {
      fprintf(stderr, "unknown variable \"%s\"\n", var_name);
      exit(1);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = tok;
    node->var = lvar;
    *rest = tok->next;
    return node;
  }

  fprintf(stderr, "unknown primary: %s\n", tok->loc);
  exit(1);
}

void assign_lvar_offsets(Function *func) {
  int offset = 0;
  for (Var *v = lvars; v; v = v->next) {
    offset += 8;
    v->offset = offset;
  }
}

// function = type-name func-name "(" ")" "{" stmt* "}"
Function *function(Token *tok, Token **rest) {
  if (is_typename(tok) && tok->next->kind != TK_IDENT &&
      equal(tok->next->next, "(") && equal(tok->next->next->next, ")") &&
      equal(tok->next->next->next->next, "{")) {
    fprintf(stderr, "function name expected: %s\n", tok->loc);
    exit(1);
  }
  Type *ty = type_with_name(tok, &tok);
  tok = tok->next->next->next; // skip "(" and ")"

  // Reset local variables list.
  lvars = NULL;

  Node head;
  Node *cur = &head;
  for (; !equal(tok, "}");)
    cur = cur->next = stmt(tok, &tok);

  Function *func = calloc(1, sizeof(Function));
  func->ty = ty;
  func->name = ty->name;
  func->body = head.next;
  func->lvars = lvars;

  assign_lvar_offsets(func);

  *rest = tok->next;
  return func;
}

// prog = function*
Function *parse(Token *tok) {
  Function head;
  Function *cur = &head;

  for (; tok->kind != TK_EOF;)
    cur = cur->next = function(tok, &tok);

  return head.next;
}
