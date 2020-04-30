#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************************
* ...type definitions...
*********************************************/

// ********** tokenize.c *************

typedef enum
{
  TK_RESERVED, // Keywords or punctuators
  TK_IDENT,    // Identifier
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
  int len;        // Length of the token
};

// type of local variables
typedef struct LVar LVar;
struct LVar
{
  LVar *next; // The next local variable or NULL.
  char *name; // The name of the local variable.
  int len;    // The length of the local variable.
  int offset; // The offset from RBP.
};

// ********** parse.c *************

// Kind of AST node
typedef enum
{
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // == equal to
  ND_NE,     // != not equal to
  ND_LT,     // <  less than
  ND_LE,     // <= less than or equal to
  ND_ASSIGN, // = assignment
  ND_LVAR,   // local variables
  ND_NUM,    // integer
} NodeKind;

// Node of AST
typedef struct Node Node;
struct Node
{
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;    // Use only if kind is ND_NUM
  int offset; // Use only if kind is ND_LVAR
};

/*********************************************
* ...global variables...
*********************************************/

// Current token
extern Token *token;

// Input program
extern char *user_input;

// Statement Nodes
extern Node *code[100];

// Local variables
extern LVar *locals;

/*********************************************
* ...function declarations...
*********************************************/

// ********** tokenize.c *************

// Report error
// Take the same arguments as printf()
void error(char *fmt, ...);

// Report error and error position
void error_at(char *loc, char *fmt, ...);

// If next token is the symbol which we expect,
// we move it forward and return true.
// Otherwise we return false
bool consume(char *op);

// If next token is a identifier,
// we move it forward and return true.
// Otherwise we return NULL.
Token *consume_ident();

// If next token is the symbol which we expect,
// we move it forward.
// Otherwise report error.
void expect(char *op);

int expect_number();

bool is_number();

bool at_eof();

// Search variable name. Return NULL if not found.
LVar *find_lvar(Token *tok);

// convert input 'user_input' to token
Token *tokenize();

// ********** parse.c *************

void program();

// ********** codegen.c *************

void gen(Node *node);
