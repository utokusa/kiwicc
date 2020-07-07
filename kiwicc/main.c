#include "kiwicc.h"

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Invalid number of argments.");
    return 1;
  }

  // Tokenize
  Token *token = tokenize_file(argv[1]);

  // parse
  // Program *prog = program_old();
  Program *prog = parse(token);

  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    int offset = 32; // 32 for callee-saved registers
    for (VarList *vl = fn->locals; vl; vl = vl->next)
    {
      offset = align_to(offset, vl->var->ty->align);
      offset += size_of(vl->var->ty);
      vl->var->offset = offset;
    }
    fn->stack_size = align_to(offset, 16);
  }

  // Emit a .file directive for the assembler.
  printf(".file 1 \"%s\"\n", argv[1]);

  // generate code
  codegen(prog);

  return 0;
}
