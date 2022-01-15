#include <stdio.h>

int main(void) {
  printf("  .intel_syntax noprefix\n");
  printf("  .text\n");
  printf("  .globl main\n");
  printf("main:\n");
  printf("  mov rax, 42\n");
  printf("  ret\n");
  return 0;
}
