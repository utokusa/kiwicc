#include "kiwicc.h"

char *file_dir;

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Invalid number of argments.");
    return 1;
  }

  // Get file directory
  file_dir = get_dir(argv[1]);

  // Tokenize
  Token *token = tokenize_file(argv[1]);

  // Preprocess
  token = preprocess(token);

  // Parse
  // Program *prog = program_old();
  Program *prog = parse(token);

  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    // Besides local variables, callee-saved registers take 32 bytes
    // and the variable-argument save takes 48 bytes in the stack.
    int offset = fn->is_variadic ? 80 : 32;

    for (VarList *vl = fn->locals; vl; vl = vl->next)
    {
      offset = align_to(offset, vl->var->align);
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
