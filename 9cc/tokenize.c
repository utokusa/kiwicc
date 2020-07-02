#include "9cc.h"

/*********************************************
* ...tokenizer...
*********************************************/

// Input filename
static char *current_filename;

// Input string
char *current_input;

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
static void verror_at(int line_no, char *loc, char *fmt, va_list ap)
{
  // Find a line containing `loc`.
  char *line = loc;
  while (current_input < line && line[-1] != '\n')
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
  int indent = fprintf(stderr, "%s:%d:", current_filename, line_no);
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
  verror_at(line_no, loc, fmt, ap);
  exit(1);
}

// Report an error position and exit
void error_tok(Token *tok, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->line_no, tok->loc, fmt, ap);
  exit(1);
}

void warn_tok(Token *tok, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->line_no, tok->loc, fmt, ap);
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
    error("Internal error at var_len : "
          "*p should be an alphabet or underscore.");
  int cnt_len = 1;
  ++p;
  // When *p is a '\0', the following condition is false
  while (is_alnum(*p))
    ++cnt_len, ++p;
  return cnt_len;
}

// Check if p starts with a reserved keyword
static char *starts_with_reserved(char *p)
{
  // Keywords
  static char *kw[] = {
      "return", "if", "else",
      "while", "for", "sizeof",
      "int", "char", "struct", "union",
      "short", "long", "void", "_Bool", "typedef"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); ++i)
  {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len]))
      return kw[i];
  }

  // Multi-letter symbols
  static char *ops[] = {"==", "!=", "<=", ">=", "->"};
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); ++i)
  {
    if (startswith(p, ops[i]))
      return ops[i];
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
  return tok;
}

// Initialize line info for all tokens.
static void add_line_info(Token *tok)
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

// Convert input 'user_input' to token
static Token *tokenize(char *filename, char *p)
{
  current_filename = filename;
  current_input = p;
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int DUMMY_LEN = 1;
  while (*p)
  {
    // Skip line comments.
    if (startswith(p, "//"))
    {
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    // Skip block comments.
    if (startswith(p, "/*"))
    {
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "unclosed block comment");
      p = q + 2;
      continue;
    }

    // Spaces
    if (isspace(*p))
    {
      ++p;
      continue;
    }

    // Keywords or multi-letter symbols
    char *kw = starts_with_reserved(p);
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
        *p == ']')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Character literal
    if (*p == '\'')
    {
      cur = read_char_literal(cur, p);
      p += cur->len;
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

    // Numeric literal
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, DUMMY_LEN);
      char *prev_p = p;
      cur->val = strtol(p, &p, 10);
      cur->len = (int)(prev_p - p);
      continue;
    }

    // String literal
    if (*p == '"')
    {
      cur = read_string_literal(cur, p);
      p += cur->len;
      continue;
    }

    error_at(p, "Can not tokenize.");
  }
  new_token(TK_EOF, cur, p, DUMMY_LEN);
  add_line_info(head.next);
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
      error("cannot open %s: %s", path, strerror(errno));
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

Token *tokenize_file(char *path)
{
  return tokenize(path, read_file(path));
}