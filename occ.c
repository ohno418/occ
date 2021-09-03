#include "occ.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "only one argumet required\n");
    exit(1);
  }

  Token *tok = tokenize(argv[1]);
  Node *node = parse(tok);
  codegen(node);

  exit(0);
}
