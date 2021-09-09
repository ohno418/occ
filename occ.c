#include "occ.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "only one argument required\n");
    exit(1);
  }

  Token *tok = tokenize(argv[1]);
  Function *prog = parse(tok);
  codegen(prog);

  exit(0);
}
