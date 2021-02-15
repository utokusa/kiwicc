#include "kiwicc.h"

static FILE *tmp_file;
static char *input_path;
char *output_path = "-";
static char *tmp_file_path;

bool opt_fpic = true;

char **include_paths;
static bool opt_E;
bool opt_MD;

void println(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(tmp_file, fmt, ap);
  fprintf(tmp_file, "\n");
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
  // to ./include relative to argv[0] (kiwicc path).
  char *kiwicc_abs_path;
  if (*argv0 == '/')
  {
    kiwicc_abs_path = argv0;
  }
  else
  {
    char cwd[PATHNAME_SIZE]; 
    memset(cwd, '\0', PATHNAME_SIZE); 
    getcwd(cwd, PATHNAME_SIZE);
    kiwicc_abs_path = rel_to_abs(cwd, argv0);
  }

  // Add include path

  // Add kiwicc specific include path (for devlopment)
  char *kiwicc_abs_dir = dirname(strdup(kiwicc_abs_path));
  char *kiwicc_include = rel_to_abs(kiwicc_abs_dir, "./include");
  add_include_path(kiwicc_include);

  // Add kiwicc specific include path (for installed binary)
  add_include_path("/opt/riscv/lib/kiwicc/include");

  // Add standard include paths of gcc.
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

    if (!strcmp(argv[i], "-Wall"))
    {
      // ignore
      continue;
    }

    if (!strcmp(argv[i], "-Werror"))
    {
      // ignore
      continue;
    }

    if (!strncmp(argv[i], "-O", 2))
    {
      // ignore code generation options
      continue;
    }

    if (!strcmp(argv[i], "-fno-omit-frame-pointer"))
    {
      // ignore
      continue;
    }
    
    if (!strcmp(argv[i], "-ggdb"))
    {
      // ignore
      continue;
    }

    if (!strcmp(argv[i], "-MD"))
    {
      opt_MD = true;
      continue;
    }

    if (!strncmp(argv[i], "-I", 2))
    {
      char *path = argv[i] + 2;
      // `path` is absolute path
      if (*path == '/')
      {
        add_include_path(path);
        continue;
      }
      // `path` is relative path
      char cwd[PATHNAME_SIZE]; 
      memset(cwd, '\0', PATHNAME_SIZE); 
      getcwd(cwd, PATHNAME_SIZE);
      char *include_path = rel_to_abs(cwd, path);
      add_include_path(include_path);
      continue;
    }

    if (argv[i][0] == '-' && argv[i][1] != '\0')
      error("unknown argument: %s", argv[i]);

    input_path = argv[i];
    if (*input_path != '/')
    {
      // Convert relative path to to absolute path
      char cwd[PATHNAME_SIZE]; 
      memset(cwd, '\0', PATHNAME_SIZE); 
      getcwd(cwd, PATHNAME_SIZE);
      input_path = rel_to_abs(cwd, input_path);
    } 
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

static void copy_file(FILE *in, FILE *out)
{
  char buf[4096];
  for (;;)
  {
    int nr = fread(buf, 1, sizeof(buf), in);
    if (nr == 0)
      break;
    fwrite(buf, 1, nr, out);
  }
}

static void cleanup()
{
  if (tmp_file_path)
    unlink(tmp_file_path);
}

int main(int argc, char **argv)
{
  add_default_include_paths(argv[0]);
  parse_args(argc, argv);
  atexit(cleanup);

  // Open a tmporary output file.
  tmp_file_path = strdup("/tmp/kiwicc-XXXXXX");
  int fd = mkstemp(tmp_file_path);
  if (!fd)
    error("cannot open output file: %s: %s", tmp_file_path, strerror(errno)); 
  tmp_file = fdopen(fd, "w");

  // Tokenize
  Token *token = tokenize_file(input_path);
  if (!token)
    error("cannot open %s: %s", argv[1], strerror(errno));

  // Preprocess
  token = preprocess(token);

  if (opt_MD)
    output_dependencies();

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

  // Write assembly to an output file.
  fseek(tmp_file, 0, SEEK_SET);

  FILE *out;
  if (strcmp(output_path, "-") == 0)
  {
    out = stdout;
  }
  else
  {
    out = fopen(output_path, "w");
    if (!out)
      error("cannot open output file: %s: %s", output_path, strerror(errno));
  }
  copy_file(tmp_file, out);

  return 0;
}
