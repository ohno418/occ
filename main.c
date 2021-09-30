#include "occ.h"

char *read_file(int argc, char **argv) {
  FILE *fp;
  if (strncmp(argv[1], "-", strlen(argv[1])) == 0) {
    fp = stdin;
  } else {
    fp = fopen(argv[1], "r");
    if (!fp) {
      fprintf(stderr, "cannot open %s: %s\n", argv[1], strerror(errno));
      exit(1);
    }
  }

  char *buf;
  size_t buflen;
  FILE *out = open_memstream(&buf, &buflen);

  for (;;) {
    char buftmp[4096];
    int nread = fread(buftmp, 1, sizeof(buftmp), fp);
    if (nread == 0)
      break;
    fwrite(buftmp, 1, nread, out);
  }

  if (fp != stdin)
    fclose(fp);

  // Make sure the input string terminated with '\0'.
  fflush(out);
  fputc('\0', out);
  fclose(out);
  return buf;
}

int main(int argc, char **argv) {
  char *input = read_file(argc, argv);
  Token *tok = tokenize(input);
  Function *prog = parse(tok);
  codegen(prog);

  exit(0);
}
