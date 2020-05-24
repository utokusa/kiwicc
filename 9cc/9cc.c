#include "9cc.h"

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
  Token *token = tokenize();

  // parse
  // Program *prog = program_old();
  Program *prog = parse(token);

  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next)
    {
      offset += vl->var->ty->size;
      vl->var->offset = offset;
    }
    fn->stack_size = offset;
  }

  // generate code
  codegen(prog);

  return 0;
}
