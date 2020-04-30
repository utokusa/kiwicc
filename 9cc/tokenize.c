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

// Report error and error position
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// If next token is the symbol which we expect,
// we move it forward and return true.
// Otherwise we return false
bool consume(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// If next token is a identifier,
// we move it forward and return token.
// Otherwise we return NULL.
Token *consume_ident()
{
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

// If next token is the symbol which we expect,
// we move it forward.
// Otherwise report error.
void expect(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "The token is not %s.", op);
  token = token->next;
}

int expect_number()
{
  if (token->kind != TK_NUM)
    error_at(token->str, "The token is not a number.");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
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

// Search variable name. Return NULL if not found.
LVar *find_lvar(Token *tok)
{
  for (LVar *var = locals; var; var = var->next)
  {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}

// Create new token and set it to the next of tok
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
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

    // symbols
    if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0)
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' ||
        *p == '<' || *p == '>' ||
        *p == '=' || *p == ';')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // keywords
    if (startswith(p, "return") && !is_alnum(p[6]))
    {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
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