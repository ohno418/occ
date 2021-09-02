#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "only one argumet required\n");
    exit(1);
  }

  printf("    .intel_syntax noprefix\n");
  printf("    .globl main\n");
  printf("main:\n");
  printf("    mov rax, %s\n", argv[1]);
  printf("    ret\n");

  exit(0);
}
