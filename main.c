#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "one argument required\n");
    exit(1);
  }

  printf("  .intel_syntax noprefix\n");
  printf("  .text\n");
  printf("  .globl main\n");
  printf("main:\n");
  printf("  mov rax, %s\n", argv[1]);
  printf("  ret\n");
  return 0;
}
