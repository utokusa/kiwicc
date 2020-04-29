#include "9cc.h"

// Current token
Token *token;

// Input program
char *user_input;

// Statements
Node *code[100];

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Invalid number of argments.");
    return 1;
  }

  // Tokenize
  user_input = argv[1];
  token = tokenize();
  program();

  // Output the assembly code.
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Prologue
  // Allocate 26 local variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n"); // 26 * 8 = 208

  {
    int i = 0;
    while (code[i])
    {
      gen(code[i++]);

      // Pop unnecessary　evaluation result of the expression.
      printf(" pop rax\n");
    }
  }

  // Epilogue
  // The value of rax is the return value
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
