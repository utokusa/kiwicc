#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
  TK_RESERVED, // Keywords or punctuators
  TK_NUM,      // Integer literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token
{
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TK_NUM, its value
  char *str;      // Token string
};

// Current token
Token *token;

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

// If next token is the symbol which we expect,
// we move it forward and return true.
// Otherwise we return false
bool consume(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// If next token is the symbol which we expect,
// we move it forward.
// Otherwise report error.
void expect(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error("The token is not %c.", op);
  token = token->next;
}

int expect_number()
{
  if (token->kind != TK_NUM)
    error("The token is not a number.");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

// create new token and set it to the next of tok
Token *new_token(TokenKind kind, Token *cur, char *str)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// convert input *p to token
Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    if (isspace(*p))
    {
      ++p;
      continue;
    }

    if (*p == '+' || *p == '-')
    {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("Can not tokenize.");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Invalid number of argments.");
    return 1;
  }

  // Tokenize
  token = tokenize(argv[1]);

  // Output the assembly code.
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  mov rax, %d\n", expect_number());

  while (!at_eof())
  {
    if (consume('+'))
    {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
