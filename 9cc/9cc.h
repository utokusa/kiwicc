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

// Type of local variables
typedef struct LVar LVar;
struct LVar
{
  char *name; // The name of the local variable.
  int len;    // The length of the local variable.
  int offset; // The offset from RBP.
};

// Linked list for Lvar
typedef struct VarList VarList;
struct VarList
{
  VarList *next;
  LVar *lvar;
};

// ********** parse.c *************

// Kind of AST node
typedef enum
{
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_EQ,      // == equal to
  ND_NE,      // != not equal to
  ND_LT,      // <  less than
  ND_LE,      // <= less than or equal to
  ND_ASSIGN,  // = assignment
  ND_ADDR,    // & address-of
  ND_DEREF,   // * dereference (indirection)
  ND_LVAR,    // local variables
  ND_NUM,     // integer
  ND_IF,      // "if"
  ND_WHILE,   // "while"
  ND_FOR,     // "for"
  ND_BLOCK,   // {...} block
  ND_RETURN,  // "return"
  ND_FUNCALL, // function call
} NodeKind;

// Node of AST
typedef struct Node Node;
struct Node
{
  NodeKind kind;

  Node *lhs;
  Node *rhs;

  Node *next;

  // "if", "while" or "for" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // {...} block
  Node *block;

  // Function call
  char *funcname;
  Node *arg;

  int val;    // Use only if kind is ND_NUM
  int offset; // Use only if kind is ND_LVAR
};

// Function
typedef struct Function Function;
struct Function
{
  Function *next;
  char *name;
  Node *node;
  VarList *locals;
  VarList *params;
  int stack_size;
};

/*********************************************
* ...global variables...
*********************************************/

// Current token
extern Token *token;

// Input program
extern char *user_input;

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
char *consume_ident();

// If next token is the symbol which we expect,
// we move it forward.
// Otherwise report error.
void expect(char *op);

int expect_number();

// If next token is an identifier,
// we move it forward.
// Otherwise report error.
char *expect_ident();

bool is_number();

bool at_eof();

// convert input 'user_input' to token
Token *tokenize();

// ********** parse.c *************

Function *program();

// ********** codegen.c *************

void codegen(Function *prog);
