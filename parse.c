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

void consume(Token **rest, char *str) {
  if (!equal(*rest, str)) {
    fprintf(stderr, "expected \"%s\": %s\n", str, (*rest)->loc);
    exit(1);
  }

  *rest = (*rest)->next;
}

// List of local vars of current function.
Var *lvars;
// List of global vars.
Var *gvars;

void register_lvar(Var *var) {
  var->next = lvars;
  lvars = var;
}

void register_gvar(Var *var) {
  var->next = gvars;
  gvars = var;
}

Var *new_lvar(Type *ty, char *name, bool is_arg) {
  Var *var = calloc(1, sizeof(Var));
  var->ty = ty;
  var->name = name;
  var->is_arg = is_arg;
  register_lvar(var);
  return var;
}

Var *find_var(char *name) {
  // find from local variables
  for (Var *v = lvars; v; v = v->next)
    if (strlen(v->name) == strlen(name) &&
        strncmp(v->name, name, strlen(name)) == 0)
      return v;

  // find from global variables
  for (Var *v = gvars; v; v = v->next)
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

Node *new_add_node(Node *lhs, Node *rhs, Token *tok) {
  // pointer + number
  if (lhs->kind == ND_VAR && lhs->var->ty->kind == TY_PTR)
    rhs = new_binary_node(ND_MUL, rhs,
        new_num_node(lhs->var->ty->base->size, NULL), tok);

  return new_binary_node(ND_ADD, lhs, rhs, tok);
}

Node *new_sub_node(Node *lhs, Node *rhs, Token *tok) {
  // pointer - number
  if (lhs->kind == ND_VAR && lhs->var->ty->kind == TY_PTR)
    rhs = new_binary_node(ND_MUL, rhs,
        new_num_node(lhs->var->ty->base->size, NULL), tok);

  return new_binary_node(ND_SUB, lhs, rhs, tok);
}

Type *is_typename(Token *tok) {
  if (equal(tok, "int"))
    return ty_int();

  if (equal(tok, "char"))
    return ty_char();

  return NULL;
}

// type_with_name = type-name "*"? var-name? ("[" num "]")?
Type *type_with_name(Token *tok, Token **rest) {
  Type *ty = is_typename(tok);
  if (!ty) {
    fprintf(stderr, "not type name: %s\n", tok->loc);
    exit(1);
  }
  tok = tok->next;

  if (equal(tok, "*")) {
    ty = ty_ptr(ty);
    tok = tok->next;
  }

  if (tok->kind == TK_IDENT) {
    ty->name = strndup(tok->loc, tok->len);
    tok = tok->next;
  }

  if (equal(tok, "[")) {
    tok = tok->next;
    if (tok->kind != TK_NUM) {
      fprintf(stderr, "expected a number: %s\n", tok->loc);
      exit(1);
    }

    char *name = ty->name;
    ty = ty_array(ty, atoi(strndup(tok->loc, tok->len)));
    ty->name = name;
    tok = tok->next;
    consume(&tok, "]");
  }

  *rest = tok;
  return ty;
}

Node *stmt(Token *tok, Token **rest);
Node *expr(Token *tok, Token **rest);
Node *assign(Token *tok, Token **rest);
Node *logical(Token *tok, Token **rest);
Node *relation(Token *tok, Token **rest);
Node *add(Token *tok, Token **rest);
Node *mul(Token *tok, Token **rest);
Node *parentheses(Token *tok, Token **rest);
Node *postfix(Token *tok, Token **rest);
Node *primary(Token *tok, Token **rest);

// stmt = "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr ";" expr ";" expr) stmt
//      | "return" expr ";"
//      | "{" stmt* "}"
//      | expr ";"
Node *stmt(Token *tok, Token **rest) {
  if (equal(tok, "if")) {
    Token *start = tok;

    tok = tok->next;
    consume(&tok, "(");
    Node *cond = expr(tok, &tok);
    consume(&tok, ")");
    Node *then = stmt(tok, &tok);

    Node *els = NULL;
    if (equal(tok, "else"))
      els = stmt(tok->next, &tok);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->tok = start;
    node->cond = cond;
    node->then = then;
    node->els = els;

    *rest = tok;
    return node;
  }

  if (equal(tok, "for")) {
    Token *start = tok;
    tok = tok->next;
    consume(&tok, "(");

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    node->tok = start;

    node->init = expr(tok, &tok);
    consume(&tok, ";");
    node->cond = expr(tok, &tok);
    consume(&tok, ";");
    if (!equal(tok, ")"))
      node->inc = expr(tok, &tok);
    consume(&tok, ")");
    node->then = stmt(tok, &tok);

    *rest = tok;
    return node;
  }

  // compound statement (block)
  if (equal(tok, "{")) {
    Token *start = tok;
    tok = tok->next;

    Node head;
    Node *cur = &head;
    for (; !equal(tok, "}");)
      cur = cur->next = stmt(tok, &tok);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->tok = start;
    node->body = head.next;

    *rest = tok->next;
    return node;
  }

  if (equal(tok, "return")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->tok = tok;
    node->lhs = expr(tok->next, &tok);
    consume(&tok, ";");
    *rest = tok;
    return node;
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT;
  node->tok = tok;
  node->lhs = expr(tok, &tok);
  consume(&tok, ";");
  *rest = tok;
  return node;
}

// expr = assign
Node *expr(Token *tok, Token **rest) {
  return assign(tok, rest);
}

// assign = logical ("=" assign)*
Node *assign(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = logical(tok, &tok);

  for (; equal(tok, "=");) {
    node = new_binary_node(ND_ASSIGN,
        node, assign(tok->next, &tok), start);
  }

  *rest = tok;
  return node;
}

// logical = relation ("&&" relation | "||" relation)*
Node *logical(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = relation(tok, &tok);

  for (;;) {
    if (equal(tok, "&&")) {
      node = new_binary_node(ND_AND, node, relation(tok->next, &tok), start);
      continue;
    }

    if (equal(tok, "||")) {
      node = new_binary_node(ND_OR, node, relation(tok->next, &tok), start);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// relation = add ("==" add | "!=" add | "<" add | ">" add
//                                     | "<=" add | ">=" add)?
Node *relation(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = add(tok, &tok);

  if (equal(tok, "=="))
    node = new_binary_node(ND_EQ, node, add(tok->next, &tok), start);

  if (equal(tok, "!="))
    node = new_binary_node(ND_NEQ, node, add(tok->next, &tok), start);

  if (equal(tok, "<"))
    node = new_binary_node(ND_LT, node, add(tok->next, &tok), start);

  if (equal(tok, ">"))
    node = new_binary_node(ND_LT, add(tok->next, &tok), node, start);

  if (equal(tok, "<="))
    node = new_binary_node(ND_LTE, node, add(tok->next, &tok), start);

  if (equal(tok, ">="))
    node = new_binary_node(ND_LTE, add(tok->next, &tok), node, start);

  *rest = tok;
  return node;
}

// add = mul ("+" mul | "-" mul)*
Node *add(Token *tok, Token **rest) {
  Node *node = mul(tok, &tok);

  for (;;) {
    if (equal(tok, "+")) {
      Token *op_tok = tok;
      node = new_add_node(node, mul(tok->next, &tok), op_tok);
      continue;
    }

    if (equal(tok, "-")) {
      Token *op_tok = tok;
      node = new_sub_node(node, mul(tok->next, &tok), op_tok);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// mul = parentheses ("*" parentheses | "/" parentheses)*
Node *mul(Token *tok, Token **rest) {
  Node *node = parentheses(tok, &tok);

  for (;;) {
    if (equal(tok, "*")) {
      node = new_binary_node(ND_MUL, node, parentheses(tok->next, &tok), tok);
      continue;
    }

    if (equal(tok, "/")) {
      node = new_binary_node(ND_DIV, node, parentheses(tok->next, &tok), tok);
      continue;
    }

    break;
  }

  *rest = tok;
  return node;
}

// parentheses = "(" expr ")"
//             | postfix
Node *parentheses(Token *tok, Token **rest) {
  if (equal(tok, "(")) {
    Node *node = expr(tok->next, &tok);
    consume(&tok, ")");
    *rest = tok;
    return node;
  }

  return postfix(tok, rest);
}

// postfix = primary ("++" | "--")?
Node *postfix(Token *tok, Token **rest) {
  Token *start = tok;
  Node *node = primary(tok, &tok);

  if (equal(tok, "++")) {
    node = new_binary_node(
        ND_ASSIGN,
        node,
        new_add_node(
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
        new_sub_node(
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

// func_call = ident "(" args* ")"
// args      = primary ("," primary)*
Node *func_call(Token *tok, Token **rest) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCALL;
  node->tok = tok;
  node->func_name = strndup(tok->loc, tok->len);;
  tok = tok->next->next;

  Node head = {};
  Node *cur = &head;
  for (int i = 0; !equal(tok, ")"); i++) {
    if (i != 0)
      consume(&tok, ",");

    cur = cur->next = expr(tok, &tok);
  }
  node->args = head.next;

  *rest = tok->next;
  return node;
}

// primary = number
//         | char
//         | ident "(" ")"
//         | "sizeof" "(" ident ")"
//         | "&" primary
//         | "*" parentheses
//         | type-name ident
//         | ident
//         | (null)
Node *primary(Token *tok, Token **rest) {
  // number
  if (tok->kind == TK_NUM) {
    Node *node = new_num_node(atoi(strndup(tok->loc, tok->len)), tok);
    *rest = tok->next;
    return node;
  }

  // char
  if (tok->kind == TK_CHAR) {
    char c = strndup(tok->loc, tok->len)[1];
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CHAR;
    node->tok = tok;
    node->num = c;
    *rest = tok->next;
    return node;
  }

  // function call
  if (tok->kind == TK_IDENT && equal(tok->next, "("))
    return func_call(tok, rest);

  // sizeof
  if (equal(tok, "sizeof")) {
    Token *start = tok;
    tok = tok->next;

    consume(&tok, "(");

    Token *operand_tok = tok;
    Node *node;
    if (is_typename(operand_tok)) {
      // type
      Type *ty = type_with_name(tok, &tok);
      node = new_num_node(ty->size, start);
    } else {
      // variable
      char *var_name = strndup(operand_tok->loc, operand_tok->len);
      Var *var = find_var(var_name);
      if (!var) {
        fprintf(stderr, "unknown variable \"%s\"\n", var_name);
        exit(1);
      }
      node = new_num_node(var->ty->size, start);
      tok = tok->next;
    }

    consume(&tok, ")");
    *rest = tok;
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
    Node *lhs = parentheses(tok->next, &tok);

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
    Var *var = find_var(ty->name);
    if (var) {
      fprintf(stderr, "variable \"%s\" is already declared\n", ty->name);
      exit(1);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = start;
    node->var = new_lvar(ty, ty->name, false);

    *rest = tok;
    return node;
  }

  // existing variable
  if (tok->kind == TK_IDENT) {
    char *var_name = strndup(tok->loc, tok->len);
    Var *var = find_var(var_name);
    if (!var) {
      fprintf(stderr, "unknown variable \"%s\"\n", var_name);
      exit(1);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->tok = tok;
    node->var = var;
    *rest = tok->next;
    return node;
  }

  if (equal(tok, ";"))
    return NULL;

  fprintf(stderr, "unknown primary: %s\n", tok->loc);
  exit(1);
}

bool is_gvar(Token *tok) {
  type_with_name(tok, &tok);
  return !equal(tok, "(");
}

// gvar = type-name ident ";"
void gvar(Token *tok, Token **rest) {
  Type *ty = type_with_name(tok, &tok);
  Var *var = calloc(1, sizeof(Var));
  var->name = ty->name;
  var->ty = ty;
  var->is_global = true;
  register_gvar(var);

  consume(&tok, ";");
  *rest = tok;
}

void assign_lvar_offsets(Function *func) {
  int offset = 0;
  for (Var *v = lvars; v; v = v->next) {
    offset += v->ty->size;
    v->offset = offset;
  }
}

// func_args = (type-name ident ("," type-name ident)*)?
void func_args(Token *tok, Token **rest) {
  for (int i = 0; !equal(tok, ")"); i++) {
    if (i != 0)
      consume(&tok, ",");

    Type *ty = type_with_name(tok, &tok);
    new_lvar(ty, ty->name, true);
  }

  *rest = tok;
}

// function = type-name func-name "(" func_args ")" "{" stmt* "}"
Function *function(Token *tok, Token **rest) {
  Type *ty = type_with_name(tok, &tok);
  char *func_name = ty->name;
  consume(&tok, "(");

  // Reset local variables list.
  lvars = NULL;

  func_args(tok, &tok);
  consume(&tok, ")");
  consume(&tok, "{");

  Node head;
  Node *cur = &head;
  for (; !equal(tok, "}");)
    cur = cur->next = stmt(tok, &tok);

  Function *func = calloc(1, sizeof(Function));
  func->ty = ty;
  func->name = func_name;
  func->body = head.next;
  func->lvars = lvars;

  assign_lvar_offsets(func);

  consume(&tok, "}");
  *rest = tok;
  return func;
}

// prog = (gvar | function)*
Function *parse(Token *tok) {
  Function head;
  Function *cur = &head;

  for (; tok->kind != TK_EOF;) {
    if (is_gvar(tok)) {
      gvar(tok, &tok);
      continue;
    }

    cur = cur->next = function(tok, &tok);
  }

  return head.next;
}
