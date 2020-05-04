#include "9cc.h"

// Current token
Token *token;

// Input program
char *user_input;

// Statements
Node *code;

// Local Variables
LVar *locals;

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
  // Allocate local variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  int stack_size = 0;
  if (locals)
    stack_size = locals->offset;
  printf("  sub rsp, %d\n", stack_size);

  {
    for (Node *cur = code; cur; cur = cur->next)
    {
      gen(cur);
      // Pop unnecessary　evaluation result of the expression.
      printf("  pop rax\n");
    }
  }

  // Epilogue
  // The value of rax is the return value
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
