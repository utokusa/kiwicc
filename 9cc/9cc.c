#include "9cc.h"

// Current token
Token *token;

// Input program
char *user_input;

// Local Variables
VarList *locals;

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

  // parse
  Function *prog = program();

  // generate code
  codegen(prog);

  return 0;
}
