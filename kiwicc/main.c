#include "kiwicc.h"

char *file_dir;
static FILE *output_file;
static char *input_path;
static char *output_path = "-";

bool opt_fpic = true;

char **include_paths;
static bool opt_E;

void println(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(output_file, fmt, ap);
  fprintf(output_file, "\n");
}

static void usage(int status)
{
  fprintf(stderr, "kiwicc [ -o <path> ] [ -fpic | -fno-pic ] [ -E ] <file>\n");
  exit(status);
}

static void add_include_path(char *path)
{
  static int len = 2;
  include_paths = realloc(include_paths, sizeof(char *) * len);
  include_paths[len - 2] = path;
  include_paths[len - 1] = NULL;
  len ++;
}

static void add_default_include_paths(char *argv0)
{
  // We expect that kiwicc-specific include files are installed
  // to ./include relative to argv[0];
  char *buf = calloc(1, strlen(argv0) + 10);
  sprintf(buf, "%s/include", dirname(strdup(argv0)));
  add_include_path(buf);

  // Add standard include paths.
  // You can get these paths by "echo | riscv64-unknown-linux-gnu-gcc -E -Wp,-v -".
  add_include_path("/opt/riscv/lib/gcc/riscv64-unknown-linux-gnu/10.2.0/include");
  add_include_path("/opt/riscv/lib/gcc/riscv64-unknown-linux-gnu/10.2.0/include-fixed");
  add_include_path("/opt/riscv/lib/gcc/riscv64-unknown-linux-gnu/10.2.0/../../../../riscv64-unknown-linux-gnu/include");
  add_include_path("/opt/riscv/sysroot/usr/include");
  
}

static void parse_args(int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--help"))
      usage(0);
    
    if (!strcmp(argv[i], "-o"))
    {
      if (!argv[++i])
        usage(1);
      output_path = argv[i];
      continue;
    }

    if (!strncmp(argv[i], "-o", 2))
    {
      output_path = argv[i] + 2;
      continue;
    }

    if (!strcmp(argv[i], "-fpic")|| !strcmp(argv[i], "-fPIC"))
    {
      opt_fpic = true;
      continue;
    }

    if (!strcmp(argv[i], "-fno-pic") || !strcmp(argv[i], "-fno-PIC"))
    {
      opt_fpic = false;
      continue;
    }

    if (!strcmp(argv[i], "-E"))
    {
      opt_E = true;
      continue;
    }

    if (!strncmp(argv[i], "-I", 2))
    {
      add_include_path(argv[i] + 2);
      continue;
    }

    if (argv[i][0] == '-' && argv[i][1] != '\0')
      error("unknown argument: %s", argv[i]);

    input_path = argv[i];
  }

  if (!input_path)
    error("no input files");
}

// Print tokens to stdout. Used for -E.
static void print_tokens(Token *tok)
{
  int line = 1;
  for (; tok->kind != TK_EOF; tok = tok->next)
  {
    if (line > 1 && tok->at_bol)
      printf("\n");
    if (tok->has_space && !tok->at_bol)
      printf(" ");
    printf(" %.*s", tok->len, tok->loc);
    line++;
  }
  printf("\n");
}

int main(int argc, char **argv)
{
  add_default_include_paths(argv[0]);
  parse_args(argc, argv);

  // Open the output file.
  if (strcmp(output_path, "-") == 0)
  {
    output_file = stdout;
  }
  else
  {
    output_file = fopen(output_path, "w");
    if (!output_file)
      error("cannot open output file: %s: %s", output_path, strerror(errno));
  }

  // Get file directory
  file_dir = get_dir(input_path);

  // Tokenize
  Token *token = tokenize_file(input_path);
  if (!token)
    error("cannot open %s: %s", argv[1], strerror(errno));

  // Preprocess
  token = preprocess(token);

  // If -E is given, print out preprocessed C code as a result
  if (opt_E)
  {
    print_tokens(token);
    exit(0);
  }

  // Parse
  // Program *prog = program_old();
  Program *prog = parse(token);

  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    // Besides local variables, callee-saved registers take 23 bytes
    // and the variable-argument save takes 31 bytes in the stack.
    int offset = fn->is_variadic ? 248 : 184;

    for (VarList *vl = fn->locals; vl; vl = vl->next)
    {
      offset = align_to(offset, vl->var->align);
      offset += size_of(vl->var->ty);
      vl->var->offset = offset;
    }
    fn->stack_size = align_to(offset, 16);
  }

  // generate code
  codegen(prog);

  return 0;
}
