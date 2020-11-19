#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/*********************************************
* ...type definitions...
*********************************************/

typedef struct Type Type;
typedef struct Hideset Hideset;
typedef struct Member Member;
typedef struct Relocation Relocation;

typedef enum
{
  TK_RESERVED, // Keywords or punctuators
  TK_IDENT,    // Identifier
  TK_STR,      // String literrals
  TK_NUM,      // Integer literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token
{
  TokenKind kind; // Token kind
  Token *next;    // Next token
  long val;       // If kind is TK_NUM, its value
  char *loc;      // Token location
  int len;        // Length of the token

  char *contents; // String literal contents including terminating '\0'
  int cont_len;   // String literal length

  Type *ty;       // Used if TK_NUM

  char *filename;   // input filename
  char *input;      // Entire input string
  int file_no;      // File number for .loc directive
  int line_no;      // Line number
  bool at_bol;      // True if this token is at beginning of line
  Hideset *hideset; // For macro expansionl
};

// Variable
typedef struct Var Var;
struct Var
{
  char *name;    // The name of the local variable.
  Type *ty;      // Type
  Token *tok;    // representative token
  bool is_local; // local or global
  int align;     // alignment

  // Local variable
  int offset; // The offset from RBP.

  // Global variable
  char *init_data;
  Relocation *rel;
  bool is_static;
};

// Global variable can be initialized either by a constant expression
// or a pointer to another global variable. This struct represents the latter.
typedef struct Relocation Relocation;
struct Relocation
{
  Relocation *next;
  int offset;
  char *label;
  long addend;
};

// Linked list for Lvar
typedef struct VarList VarList;
struct VarList
{
  VarList *next;
  Var *var;
};

// Kind of AST node
typedef enum
{
  ND_ADD,       // num + num
  ND_PTR_ADD,   // ptr + num or num + ptr
  ND_SUB,       // num - num
  ND_PTR_SUB,   // ptr - num
  ND_PTR_DIFF,  // ptr - ptr
  ND_MUL,       // *
  ND_DIV,       // /
  ND_MOD,       // %
  ND_BITAND,    // &
  ND_BITOR,     // |
  ND_BITXOR,    // ^
  ND_SHL,       // <<
  ND_SHR,       // >>
  ND_LOGAND,    // &&
  ND_LOGOR,     // ||
  ND_EQ,        // == equal to
  ND_NE,        // != not equal to
  ND_LT,        // <  less than
  ND_LE,        // <= less than or equal to
  ND_ASSIGN,    // = assignment
  ND_COND,      // ?: conditional operator
  ND_COMMA,     // , commma operator
  ND_MEMBER,    // . struct member access
  ND_ADDR,      // & address-of
  ND_DEREF,     // * dereference (indirection)
  ND_NOT,       // ! not
  ND_BITNOT,    // ~ bitwise not
  ND_VAR,       // variable
  ND_NUM,       // integer
  ND_CAST,      // type cast
  ND_IF,        // "if"
  ND_WHILE,     // "while"
  ND_DO,        // "do"
  ND_FOR,       // "for"
  ND_SWITCH,    // "switch"
  ND_CASE,      // "case"
  ND_BLOCK,     // {...} block
  ND_BREAK,     // "break"
  ND_CONTINUE,  // "continue"
  ND_GOTO,      // "goto"
  ND_LABEL,     // Labeled statement
  ND_RETURN,    // "return"
  ND_FUNCALL,   // function call
  ND_EXPR_STMT, // expression statement
  ND_STMT_EXPR, // statement expression
  ND_NULL_EXPR, // do nothing
} NodeKind;

// Node of AST
typedef struct Node Node;
struct Node
{
  NodeKind kind;
  Token *tok; // Representative token
  Type *ty;   // TYpe, e.g. int or pointer to int

  Node *lhs;
  Node *rhs;

  Node *next;

  // "if", "while" or "for" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // Assignment
  bool is_init;

  // Block or statement expression
  Node *body;

  // Struct member access
  Member *member;

  // Function call
  char *funcname;
  Type *func_ty;
  Var **args;
  int nargs;

  // Goto or labeled statement
  char *label_name;

  // Switch-cases
  Node *case_next;
  Node *default_case;
  int case_label;

  // Variable
  Var *var;

  // Numeric
  long val;
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
  bool is_static;
  bool is_variadic;
  int stack_size;
};

// Program
typedef struct Program Program;
struct Program
{
  VarList *globals;
  Function *fns;
};

typedef enum
{
  TY_VOID,
  TY_BOOL,
  TY_CHAR,
  TY_SHORT,
  TY_INT,
  TY_LONG,
  TY_ENUM,
  TY_PTR,
  TY_FUNC,
  TY_ARR,
  TY_STRUCT,
} TypeKind;

struct Type
{
  TypeKind kind;      // type kind
  int size;           // sizeof() value
  int align;          // alignment
  bool is_unsigned;   // unsigned or signed
  bool is_const;
  bool is_incomplete; // incomplete type
  Type *base;         // base type

  // Declaration
  Token *name;
  Token *name_pos;    // position that an identifier should be

  // Array
  int array_len; // number of elements in an array

  // Struct
  Member *members;

  // Function type
  Type *return_ty;
  Type *params;
  bool is_variadic;
  Type *next;
};

// Struct member
struct Member
{
  Member *next;
  Type *ty;
  Token *tok; // for error message
  Token *name;
  int align;
  int offset;
};

/*********************************************
* ...global variables...
*********************************************/

extern Type *void_type;
extern Type *bool_type;
extern Type *char_type;
extern Type *short_type;
extern Type *int_type;
extern Type *long_type;
extern Type *uchar_type;
extern Type *ushort_type;
extern Type *uint_type;
extern Type *ulong_type;

extern char *file_dir;

/*********************************************
* ...function declarations...
*********************************************/

// ********** tokenize.c *************

// Report error
// Take the same arguments as printf()
void error(char *fmt, ...);

// Report error and error position
void error_at(char *loc, char *fmt, ...);

void error_tok(Token *tok, char *fmt, ...);

void warn_tok(Token *tok, char *fmt, ...);

bool equal(Token *tok, char *s);

Token *skip(Token *tok, char *s);

Token *copy_token(Token *tok);

char **get_input_files();

// convert input 'user_input' to token
Token *tokenize_file(char *filename);

// ********** preprocess.c *************
Token *preprocess(Token *tok);

char *get_dir(char *path);

// ********** parse.c *************
Node *new_cast(Node *expr, Type *ty);

Program *parse(Token *tok);

// ********** codegen.c *************

void codegen(Program *prog);

// ********** type.c *************

bool is_integer(Type *ty);

Type *copy_type(Type *ty);

int align_to(int n, int align);

Type *pointer_to(Type *base);

Type *func_type(Type *return_ty);

Type *array_of(Type *base, int len);

Type *enum_type();

Type *struct_type();

int size_of(Type *ty);

void add_type(Node *node);
