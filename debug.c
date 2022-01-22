#include "occ.h"

static void print_node(Node *node, int depth);

static void print_tabs(int depth) {
  for (int i = 0; i < depth; i++)
    printf("\t");
}

static void print_binary(Node *node, int depth) {
  // lhs
  print_tabs(depth);
  printf("lhs:\n");
  print_node(node->lhs, depth + 1);

  // rhs
  print_tabs(depth);
  printf("rhs:\n");
  print_node(node->rhs, depth + 1);
}

static void print_node(Node *node, int depth) {
  switch (node->kind) {
    case ND_STMT:
      print_tabs(depth);
      printf("kind: ND_STMT\n");
      print_tabs(depth);
      printf("body:\n");
      print_node(node->body, depth + 1);
      break;
    case ND_NUM:
      print_tabs(depth);
      printf("kind: ND_NUM\n");
      print_tabs(depth);
      printf("num: %d\n", node->num);
      break;
    case ND_ADD:
      print_tabs(depth);
      printf("kind: ND_ADD\n");
      print_binary(node, depth);
      break;
    case ND_SUB:
      print_tabs(depth);
      printf("kind: ND_SUB\n");
      print_binary(node, depth);
      break;
    case ND_MUL:
      print_tabs(depth);
      printf("kind: ND_MUL\n");
      print_binary(node, depth);
      break;
    case ND_DIV:
      print_tabs(depth);
      printf("kind: ND_DIV\n");
      print_binary(node, depth);
      break;
    default:
      fprintf(stderr, "unknown kind of node: %d\n", node->kind);
      exit(1);
  }
}

void debug_node(Node *node) {
  printf("\n");
  printf("=== debug start ===\n");
  print_node(node, 0);
  printf("=== debug end ===\n");
  printf("\n");
}
