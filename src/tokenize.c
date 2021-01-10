#include "kiwicc.h"

/*********************************************
* ...tokenizer...
*********************************************/

// Input filename
static char *current_filename;
static char *current_filepath;

// True if the current position is at the biggining of a line.
static bool at_bol;

// Input string
char *current_input;

// A list of all input files.
static char **input_files;

// True if the current position follows a space character.
static bool has_space;

// Report error
// Take the same arguments as printf()
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Report an error message in the following format.
//
// foo.c:10: x = y + 1;
//               ^ <error message here>
static void verror_at(char *filename, char *input, int line_no, char *loc, char *fmt, va_list ap)
{
  // Find a line containing `loc`.
  char *line = loc;
  while (input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // Currently the pointers look like this
  // .
  // .     // The lines before the error
  // .
  // printf("Hello"\n
  // ^ line        ^ end (points to '\n')
  //              ^ loc
  // .
  // .     // The lines after the error
  // .

  // Print out the line.
  int indent = fprintf(stderr, "%s:%d:", filename, line_no);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // Show the error message.
  int pos = loc - line + indent;

  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

// Report an error position and exit
void error_at(char *loc, char *fmt, ...)
{
  // Get a line number
  int line_no = 1;
  for (char *p = current_input; p < loc; p++)
    if (*p == '\n')
      line_no++;

  va_list ap;
  va_start(ap, fmt);
  verror_at(current_filename, current_input, line_no, loc, fmt, ap);
  exit(1);
}

// Report an error position and exit
void error_tok(Token *tok, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->filename, tok->input, tok->line_no, tok->loc, fmt, ap);
  exit(1);
}

void warn_tok(Token *tok, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->filename, tok->input, tok->line_no, tok->loc, fmt, ap);
}

// Consumes the current token if it matches `s`.
bool equal(Token *tok, char *s)
{
  return strlen(s) == tok->len &&
         !strncmp(tok->loc, s, tok->len);
}

// Ensure that the current token is `s`.
Token *skip(Token *tok, char *s)
{
  if (!equal(tok, s))
    error_tok(tok, "expected '%s'", s);
  return tok->next;
}

Token *copy_token(Token *tok)
{
  Token *ret = malloc(sizeof(Token));
  *ret = *tok;
  return ret;
}

static bool startswith(char *tgt, char *ref)
{
  return strncmp(tgt, ref, strlen(ref)) == 0;
}

static bool is_alpha(char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c)
{
  return is_alpha(c) || isdigit(c);
}

static bool is_hex(char c)
{
  return ('0' <= c && c <= '9') ||
         ('a' <= c && c <= 'f') ||
         ('A' <= c && c <= 'F');
}

static int from_hex(char c)
{
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  return c - 'A' + 10;
}

// Return the length of the variable name starting from p.
static int var_len(char *p)
{
  if (!is_alpha(*p))
    error("Internal error at var_len : *p should be an alphabet or underscore.");
  int cnt_len = 1;
  ++p;
  // When *p is a '\0', the following condition is false
  while (is_alnum(*p))
    ++cnt_len, ++p;
  return cnt_len;
}

static bool is_keyword(Token *tok)
{
  char *p = strndup(tok->loc, tok->len);
  // Keywords
  static char *kw[] = {
      "return", "if", "else",
      "while", "do", "for", "sizeof",
      "int", "char", "struct", "union", "enum",
      "short", "long", "void", "_Bool", "typedef",
      "static", "extern", "break", "continue", "goto",
      "switch", "case", "default", "_Alignof", "_Alignas",
      "signed", "unsigned", "const",
      "volatile", "register", "restrict", "_Noreturn",
      "float", "double"
      };

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); ++i)
  {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len]))
      return true;
  }

  return false;
}

void convert_keywords(Token *tok)
{
  for (Token *t = tok; t->kind != TK_EOF; t = t->next)
  {
    if (t->kind == TK_IDENT && is_keyword(t))
      t->kind = TK_RESERVED;
  }
}

// Check if p starts with a reserved keyword
static char *starts_with_multi_letter_symbol(char *p)
{
  // Three-letter punctuators
  static char *ops3[] = {"<<=", ">>=", "..."};
  for (int i = 0; i < sizeof(ops3) / sizeof(*ops3); ++i)
  {
    if (startswith(p, ops3[i]))
      return ops3[i];
  }

  // Two-letter punctuators
  static char *ops2[] = {"==", "!=", "<=", ">=", "->", "+=", "-=", "*=", "/=",
                         "%=", "++", "--", "&=", "|=", "^=", "&&", "||",
                         "<<", ">>", "##"};
  for (int i = 0; i < sizeof(ops2) / sizeof(*ops2); ++i)
  {
    if (startswith(p, ops2[i]))
      return ops2[i];
  }

  return NULL;
}

// Create new token and set it to the next of tok
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = str;
  tok->len = len;
  tok->filename = current_filename;
  tok->filepath = current_filepath;
  tok->input = current_input;
  tok->at_bol = at_bol;
  tok->has_space = has_space;
  at_bol = has_space = false;
  cur->next = tok;
  return tok;
}

static char read_escaped_char(char **new_pos, char *p)
{
  if ('0' <= *p && *p <= '7')
  {
    // Read an octal number up to 3 digits.
    int c = *p++ - '0';
    if ('0' <= *p && *p <= '7')
    {
      c = (c << 3) | (*p++ - '0');
      if ('0' <= *p && *p <= '7')
      {
        c = (c << 3) | (*p++ - '0');
      }
    }
    *new_pos = p;
    return c;
  }

  if (*p == 'x')
  {
    // Read an hexadecimal number up to 255
    p++;
    if (!is_hex(*p))
      error_at(p, "invalid hex escape sequence");

    int c = 0;
    for (; is_hex(*p); p++)
    {
      c = (c << 4) | from_hex(*p);
      if (c > 255)
        error_at(p, "hex escape sequence out of range");
    }
    *new_pos = p;
    return c;
  }

  *new_pos = p + 1;

  switch (*p)
  {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'v':
    return '\v';
  case 'f':
    return '\f';
  case 'r':
    return '\r';
  case 'e':
    return '\e';
  default:
    return *p;
  }
}

static Token *read_string_literal(Token *cur, char *start)
{
  //  ..."abc"....
  //     ^
  //     |
  //     start
  char *p = start + 1;
  char *end = p;

  // Find the closing double-quote.
  for (; *end != '"'; end++)
  {
    if (*end == '\0')
      error_at(start, "unclosed string literal");
    if (*end == '\\')
      end++;
  }

  // Allocate a buffer that is large enough to hold the entire string.
  // ..."abc"...
  //     p
  //        ^
  //        |
  //        end
  char *buf = malloc(end - p + 1);
  int len = 0;

  while (*p != '"')
  {
    if (*p == '\\')
    {
      buf[len++] = read_escaped_char(&p, p + 1);
    }
    else
    {
      buf[len++] = *p++;
    }
  }

  buf[len++] = '\0';

  Token *tok = new_token(TK_STR, cur, start, p - start + 1);
  tok->contents = buf;
  tok->cont_len = len; // cont_len include terminating '\0'
  return tok;
}

static Token *read_char_literal(Token *cur, char *start)
{
  char *p = start + 1;
  if (*p == '\0')
    error_at(start, "unclosed char literal");

  char c;
  if (*p == '\\')
    c = read_escaped_char(&p, p + 1);
  else
    c = *p++;

  if (*p != '\'')
    error_at(p, "char literal too long");
  p++;

  Token *tok = new_token(TK_NUM, cur, start, p - start);
  tok->val = c;
  tok->ty = int_type;
  return tok;
}

// Initialize line info for all tokens.
static void add_line_numbers(Token *tok)
{
  char *p = current_input;
  int line_no = 1;

  do
  {
    if (p == tok->loc)
    {
      tok->line_no = line_no;
      tok = tok->next;
    }
    if (*p == '\n')
      line_no++;
  } while (*p++);
}

static Token *read_int_literal(Token *cur, char *start)
{
  char *p = start;

  int base = 10;
  if (!strncasecmp(p, "0x", 2) && is_alnum(p[2]))
  {
    p += 2;
    base = 16;
  }
  else if (!strncasecmp(p, "0b", 2) && is_alnum(p[2]))
  {
    p += 2;
    base = 2;
  }
  else if (*p == '0')
  {
    base = 8;
  }

  long val = strtoul(p, &p, base);

  // Read U, L or LL suffixes.
  bool l = false;
  bool u = false;

  if (startswith(p, "LLU") || startswith(p, "LLu") ||
      startswith(p, "llU") || startswith(p, "llu") ||
      startswith(p, "ULL") || startswith(p, "Ull") ||
      startswith(p, "uLL") || startswith(p, "ull"))
  {
    p += 3;
    l = u = true;    
  }
  else if (!strncasecmp(p, "lu", 2) || !strncasecmp(p, "ul", 2))
  {
    p += 2;
    l = u = true;
  }
  else if (startswith(p, "LL") || startswith(p, "ll"))
  {
    p += 2;
    l = true;
  }
  else if (*p == 'L' || *p == 'l')
  {
    p++;
    l = true;
  }
  else if (*p == 'U' || *p == 'u')
  {
    p++;
    u = true;
  }

  // Add type
  Type *ty;
  if (base == 10)
  {
    if (l && u)
      ty = ulong_type;
    else if (l)
      ty = long_type;
    else if (u)
      ty = (val >> 32) ? ulong_type : uint_type;
    else if (val >> 63)
      ty = ulong_type;
    else
      ty = (val >> 31) ? long_type : int_type;
  }
  else
  {
    if (l && u)
      ty = ulong_type;
    else if (l)
      ty = (val >> 63) ? ulong_type : long_type;
    else if (u)
      ty = (val >> 32) ? ulong_type : uint_type;
    else if (val >> 63)
      ty = ulong_type;
    else if (val >> 32)
      ty = long_type;
    else if (val >> 31)
      ty = uint_type;
    else
      ty = int_type;
  }

  Token *tok = new_token(TK_NUM, cur, start, p - start);
  tok->val = val;
  tok->ty = ty;
  return tok;
}

static Token *read_flnum_literal(Token *cur, char *start)
{
  char *end;
  double val = strtod(start, &end);

  Type *ty;
  if (*end == 'f' || *end == 'F')
  {
    ty = float_type;
    end++;
  }
  else if (*end == 'l' || *end == 'L')
  {
    ty = double_type;
    end++;
  }
  else
  {
    ty = double_type;
  }

  Token *tok = new_token(TK_NUM, cur, start, end - start);
  tok->fval =  val;
  tok->ty = ty;
  return tok;
}

static Token *read_number(Token *cur, char *start)
{
  // Try to parse as an integer constants.
  Token *tok = read_int_literal(cur, start);
  if (!strchr(".eEfF", start[tok->len]))
    return tok;
  
  // If it's not an integer, it must be a floating poiint constant.
  return read_flnum_literal(cur, start);
}

// Convert input 'user_input' to token
Token *tokenize(char *filename, int file_no, char *p)
{
  current_filename = basename(strdup(filename));
  current_filepath = filename;
  current_input = p;
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int DUMMY_LEN = 1;

  at_bol = true;
  has_space = false;

  while (*p)
  {
    // Skip line comments.
    if (startswith(p, "//"))
    {
      p += 2;
      while (*p != '\n')
        p++;
      has_space = true;
      continue;
    }

    // Skip block comments.
    if (startswith(p, "/*"))
    {
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "unclosed block comment");
      p = q + 2;
      has_space = true;
      continue;
    }

    // New line
    if (*p == '\n')
    {
      ++p;
      at_bol = true;
      has_space = false;
      continue;
    }

    // Spaces
    if (isspace(*p))
    {
      ++p;
      has_space = true;
      continue;
    }

    // Numeric literal
    if (isdigit(*p) || (p[0] == '.' && isdigit(p[1])))
    {
      cur = read_number(cur, p);
      p += cur->len;
      continue;
    }

    // Character literal
    if (*p == '\'')
    {
      cur = read_char_literal(cur, p);
      p += cur->len;
      continue;
    }

    // Wide character literal
    // TODO: currently handled as normal character literal
    if (startswith(p, "L'"))
    {
      cur = read_char_literal(cur, p + 1);
      p += cur->len + 1;
      continue;
    }

    // String literal
    if (*p == '"')
    {
      cur = read_string_literal(cur, p);
      p += cur->len;
      continue;
    }

    // Keywords or multi-letter symbols
    char *kw = starts_with_multi_letter_symbol(p);
    if (kw)
    {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // Single-letter symbols
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '{' || *p == '}' ||
        *p == '<' || *p == '>' || *p == '=' || *p == ';' ||
        *p == ',' || *p == '.' || *p == '&' || *p == '[' ||
        *p == ']' || *p == '!' || *p == '~' || *p == '%' ||
        *p == '|' || *p == '^' || *p == ':' || *p == '?' ||
        *p == '#')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Variables
    if (is_alpha(*p))
    {
      cur = new_token(TK_IDENT, cur, p, DUMMY_LEN);
      cur->len = var_len(p);
      p += cur->len;
      continue;
    }

    error_at(p, "Can not tokenize.");
  }
  new_token(TK_EOF, cur, p, DUMMY_LEN);
  for (Token *t = head.next; t; t = t->next)
    t->file_no = file_no;
  add_line_numbers(head.next);
  return head.next;
}

// Return the contents of a given file.
static char *read_file(char *path)
{
  FILE *fp;

  if (strcmp(path, "-") == 0)
  {
    // By convention, read from stdin if a given filename is "-".
    fp = stdin;
  }
  else
  {
    fp = fopen(path, "r");
    if (!fp)
      return NULL;
  }

  int buflen = 4096;
  int nread = 0;
  char *buf = malloc(buflen);

  // Read the entire file.
  for (;;)
  {
    int end = buflen - 2; // extra 2 bytes for the trailing "\n\0"
    int n = fread(buf + nread, 1, end - nread, fp);
    if (n == 0)
      break;
    nread += n;
    if (nread == end)
    {
      buflen *= 2;
      buf = realloc(buf, buflen);
    }
  }

  if (fp != stdin)
    fclose(fp);

  // Canonicalize the last line by appending "\n"
  // if it does not end with a newline.
  if (nread == 0 || buf[nread - 1] != '\n')
    buf[nread++] = '\n';
  buf[nread] = '\0';
  return buf;
}

char **get_input_files()
{
  return input_files;
}

// Remove backslashed followed by a newline.
static void remove_backslash_newline(char *p)
{
  char * q = p;

  // We want to keep the number of newline characters
  // so that the logical line number matches the physical one.
  // This counter maintain the number of newlines we have removed.
  int n = 0;

  while (*p)
  {
    if (startswith(p, "\\\n"))
    {
      p += 2;
      n ++;
    }
    else if (*p == '\n')
    {
      *q++ = *p++;
      for (; n > 0; n--)
        *q++ = '\n';
    }
    else
      *q++ = *p++;
  }

  *q = '\0';
}

Token *tokenize_file(char *path)
{
  char *p = read_file(path);
  if (!p)
    return NULL;
  
  remove_backslash_newline(p);

  // Save the filename for assembler .file directive.
  static int file_no;
  input_files = realloc(input_files, sizeof(char *) * (file_no + 2));
  input_files[file_no] = path;
  input_files[file_no + 1] = NULL;
  file_no++;

  return tokenize(path, file_no, p);
}
