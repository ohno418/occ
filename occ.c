#include "occ.h"

char *read_from_stdin() {
  char *buf;
  size_t buflen;
  FILE *out = open_memstream(&buf, &buflen);

  for (;;) {
    char buftmp[4096];
    int nread = fread(buftmp, 1, sizeof(buftmp), stdin);
    if (nread == 0)
      break;
    fwrite(buftmp, 1, nread, out);
  }
  fflush(out);

  // Make sure the input string terminated with '\0'.
  fputc('\0', out);
  fclose(out);
  return buf;
}

int main() {
  // Read input string from stdin.
  char *input = read_from_stdin();

  Token *tok = tokenize(input);
  Function *prog = parse(tok);
  codegen(prog);

  exit(0);
}
