#include "9cc.h"

/*********************************************
* ...tokenizer...
*********************************************/

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

// Report an error position and exit
static void verror_at(char *loc, char *fmt, va_list ap)
{
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Report an error position and exit
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

// Report an error position and exit
void error_tok(Token *tok, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
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
  static char *kw[] = {"return", "if", "else", "while", "for", "sizeof", "int"};
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); ++i)
  {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(len))
      return kw[i];
  }

  // Multi-letter symbols
  static char *ops[] = {"==", "!=", "<=", ">="};
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

// Convert input 'user_input' to token
Token *tokenize()
{
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int DUMMY_LEN = 1;
  while (*p)
  {
    // spaces
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
        *p == ',' || *p == '&' || *p == '[' || *p == ']')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // variables
    if (is_alpha(*p))
    {
      cur = new_token(TK_IDENT, cur, p, DUMMY_LEN);
      cur->len = var_len(p);
      p += cur->len;
      continue;
    }

    // numbers
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, DUMMY_LEN);
      char *prev_p = p;
      cur->val = strtol(p, &p, 10);
      cur->len = (int)(prev_p - p);
      continue;
    }

    error_at(p, "Can not tokenize.");
  }
  new_token(TK_EOF, cur, p, DUMMY_LEN);
  return head.next;
}