#include "occ.h"

// local variables
Var *lvars = NULL;

Var *find_lvar(char *name) {
  for (Var *v = lvars; v; v = v->next) {
    if (strlen(name) == strlen(v->name) &&
        strncmp(name, v->name, strlen(name)) == 0) {
      return v;
    }
  }
  return NULL;
}

Var *register_lvar(char *name, Type *ty) {
  // Does the name already exist?
  if (find_lvar(name)) {
    fprintf(stderr, "variable \"%s\" is already declared\n", name);
    exit(1);
  }

  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = lvars;
  var->ty = ty;
  lvars = var;
  return var;
}

bool equal(Token *tok, char *str) {
  if (!tok->loc) {
    return false;
  }

  return strncmp(tok->loc, str, strlen(str)) == 0;
}

void consume(Token *tok, Token **rest, char *str) {
  if (equal(tok, str)) {
    *rest = tok->next;
  } else {
    fprintf(stderr, "expected \"%s\": %s\n", str, tok->loc);
    exit(1);
  }
}

Node *new_num(int num, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->tok = tok;
  node->num = num;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->tok = tok;
  return node;
}

Type *type_name(Token *tok) {
  if (equal(tok, "int"))
    return ty_int();

  return NULL;
}

Node *stmt(Token *tok, Token **rest);
Node *expr(Token *tok, Token **rest);
Node *assign(Token *tok, Token **rest);
Node *add(Token *tok, Token **rest);
Node *mul(Token *tok, Token **rest);
Node *prefix(Token *tok, Token **rest);
Node *primary(Token *tok, Token **rest);

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "{" stmt* "}"
//      | ";"
//      | expr ";"
Node *stmt(Token *tok, Token **rest) {
  // return statement
  if (equal(tok, "return")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->tok = tok;
    node->body = expr(tok->next, &tok);
    consume(tok, rest, ";");
    return node;
  }

  if (equal(tok, "if")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->tok = tok;
    consume(tok->next, &tok, "(");
    node->cond = expr(tok, &tok);
    consume(tok, &tok, ")");
    node->body = stmt(tok, &tok);

    if (equal(tok, "else")) {
      node->els = stmt(tok->next, &tok);
    } else {
      node->els = NULL;
    }
    *rest = tok;
    return node;
  }

  // compound statement
  if (equal(tok, "{")) {
    Token *start = tok;
    tok = tok->next;
    Node head;
    Node *cur = &head;
    for (; !equal(tok, "}");)
      cur = cur->next = stmt(tok, &tok);
    consume(tok, rest, "}");

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->tok = start;
    node->body = head.next;
    return node;
  }

  // null statement
  if (equal(tok, ";")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NULL_STMT;
    node->tok = tok;
    node->body = NULL;
    *rest = tok->next;
    return node;
  }

  // expression statement
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_STMT;
  node->tok = tok;
  node->body = expr(tok, &tok);
  consume(tok, rest, ";");
  return node;
}

// expr = assign ("," expr)*
Node *expr(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = assign(tok, &tok);

  for (; equal(tok, ",");) {
    tok = tok->next;
    node = new_binary(ND_COMMA, node, expr(tok, &tok), start);
  }

  *rest = tok;
  return node;
}

// assign = add ("=" assign)*
Node *assign(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = add(tok, &tok);

  for (; equal(tok, "=");) {
    Node *rhs = assign(tok->next, &tok);
    node = new_binary(ND_ASSIGN, node, rhs, start);
    continue;
  }

  *rest = tok;
  return node;
}

// add = mul (("+" | "-") mul)*
Node *add(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = mul(tok, &tok);

  for (;;) {
    if (strncmp(tok->loc, "+", tok->len) == 0) {
      Node *rhs = mul(tok->next, &tok);
      node = new_binary(ND_ADD, node, rhs, start);
      continue;
    }

    if (strncmp(tok->loc, "-", tok->len) == 0) {
      Node *rhs = mul(tok->next, &tok);
      node = new_binary(ND_SUB, node, rhs, start);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// mul = prefix (("*" | "/") prefix)*
Node *mul(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = prefix(tok, &tok);

  for (; tok->kind == TK_PUNCT;) {
    if (equal(tok, "*")) {
      Node *rhs = prefix(tok->next, &tok);
      node = new_binary(ND_MUL, node, rhs, start);
      continue;
    }

    if (equal(tok, "/")) {
      Node *rhs = prefix(tok->next, &tok);
      node = new_binary(ND_DIV, node, rhs, start);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// prefix == ("++" | "--")? primary
Node *prefix(Token *tok, Token **rest) {
  if (equal(tok, "++")) {
    Token *start = tok;
    tok = tok->next;

    Node *var_node = calloc(1, sizeof(Node));
    var_node->kind = ND_VAR;
    var_node->tok = tok;
    char *varname = strndup(tok->loc, tok->len);
    var_node->var = find_lvar(varname);
    if (!var_node->var) {
      fprintf(stderr, "unkown variable: %s\n", varname);
      exit(1);
    }
    *rest = tok->next;

    return new_binary(
        ND_ASSIGN,
        var_node,
        new_binary(ND_ADD,
                   var_node,
                   new_num(1, start),
                   start),
        start);
  }

  if (equal(tok, "--")) {
    Token *start = tok;
    tok = tok->next;

    Node *var_node = calloc(1, sizeof(Node));
    var_node->kind = ND_VAR;
    var_node->tok = tok;
    char *varname = strndup(tok->loc, tok->len);
    var_node->var = find_lvar(varname);
    if (!var_node->var) {
      fprintf(stderr, "unkown variable: %s\n", varname);
      exit(1);
    }
    *rest = tok->next;

    return new_binary(
        ND_ASSIGN,
        var_node,
        new_binary(ND_SUB,
                   var_node,
                   new_num(1, start),
                   start),
        start);
  }

  return primary(tok, rest);
}

// primary = type identifier
//         | identifier "(" ")"
//         | identifier
//         | number
//         | "sizeof" "(" (identifier | type) ")"
//         | "(" expr ")"
Node *primary(Token *tok, Token **rest) {
  // declaration
  Type *ty = type_name(tok);
  if (ty) {
    tok = tok->next;
    Var *var = register_lvar(strndup(tok->loc, tok->len), ty);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = tok;
    node->var = var;
    *rest = tok->next;
    return node;
  }

  // function call
  if (tok->kind == TK_IDENT &&
      equal(tok->next, "(") && equal(tok->next->next, ")")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCALL;
    node->tok = tok;
    // didn't check if function exists
    node->func_name = strndup(tok->loc, tok->len);
    *rest = tok->next->next->next;
    return node;
  }

  // identifier
  if (tok->kind == TK_IDENT) {
    char *varname = strndup(tok->loc, tok->len);
    Var *var = find_lvar(varname);
    if (!var) {
      fprintf(stderr, "unknown local variable \"%s\": %s\n", varname, tok->loc);
      exit(1);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = tok;
    node->var = var;
    *rest = tok->next;
    return node;
  }

  // number
  if (tok->kind == TK_NUM) {
    Node *node = new_num(tok->num, tok);
    *rest = tok->next;
    return node;
  }

  // sizeof
  if (equal(tok, "sizeof")) {
    consume(tok->next, &tok, "(");
    Type *ty = NULL;
    if (tok->kind == TK_IDENT) {
      char *varname = strndup(tok->loc, tok->len);
      Var *var = find_lvar(varname);
      if (!var) {
        fprintf(stderr, "unknown local variable \"%s\": %s\n", varname, tok->loc);
        exit(1);
      }
      ty = var->ty;
    } else {
      ty = type_name(tok);
    }
    if (!ty) {
      fprintf(stderr, "unknown operand of sizeof: %s\n", tok->loc);
      exit(1);
    }
    consume(tok->next, rest, ")");

    return new_num(ty->size, tok);
  }

  // parenthesis expression
  if (equal(tok, "(")) {
    Node *node = expr(tok->next, &tok);
    consume(tok, rest, ")");
    return node;
  }

  fprintf(stderr, "unknown primary expression: %s\n", tok->loc);
  exit(1);
}

// function = type name "(" ")" "{" stmt* "}"
Function *function(Token *tok, Token **rest) {
  Function *func = calloc(1, sizeof(Function));
  lvars = NULL;

  func->ty = type_name(tok);
  if (!func->ty) {
    fprintf(stderr, "type name required for function: %s\n", tok->loc);
    exit(1);
  }
  tok = tok->next;

  if (tok->kind != TK_IDENT) {
    fprintf(stderr, "expected function name: %s\n", tok->loc);
    exit(1);
  }
  func->name = strndup(tok->loc, tok->len);
  tok = tok->next;

  consume(tok, &tok, "(");
  consume(tok, &tok, ")");
  consume(tok, &tok, "{");

  // AST of body
  Node head;
  Node *cur = &head;
  for (; !equal(tok, "}");)
    cur = cur->next = stmt(tok, &tok);

  consume(tok, rest, "}");

  func->body = head.next;
  func->vars = lvars;
  return func;
}

// program = function*
Function *parse(Token *tok) {
  Function head;
  Function *cur = &head;

  for (; tok->kind != TK_EOF;)
    cur = cur->next = function(tok, &tok);

  return head.next;
}
