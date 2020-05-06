#include "9cc.h"

// Current token
Token *token;

// Input program
char *user_input;

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

  // Assign offsets to local variables.
  for (Function *fn = prog; fn; fn = fn->next)
  {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next)
    {
      offset += 8;
      vl->lvar->offset = offset;
    }
    fn->stack_size = offset;
  }

  // generate code
  codegen(prog);

  return 0;
}
