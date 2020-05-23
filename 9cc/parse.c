#include "9cc.h"

/*********************************************
* ...parser...
*********************************************/

// Local variables
static VarList *locals;
static VarList *globals;

static Function *function();
static Type *basetype();
static void global_var();
static Node *declaration();
static Node *stmt();
static Node *stmt2();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *postfix();
static Node *primary();

static Node *new_node(NodeKind kind, Token *tok)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

static Node *new_node_num(int val, Token *tok)
{
  Node *node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}

static Node *new_node_var(Var *var, Token *tok)
{
  Node *node = new_node(ND_VAR, tok);
  node->var = var;
  return node;
}

static Node *new_node_unary(NodeKind kind, Node *expr, Token *tok)
{
  Node *node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_node_add(Node *lhs, Node *rhs, Token *tok)
{
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_node_binary(ND_ADD, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_node_binary(ND_PTR_ADD, lhs, rhs, tok);
  if (is_integer(lhs->ty) && rhs->ty->base)
    return new_node_binary(ND_PTR_ADD, lhs, rhs, tok);
  error_tok(tok, "invalid operands");
}

static Node *new_node_sub(Node *lhs, Node *rhs, Token *tok)
{
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_node_binary(ND_SUB, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_node_binary(ND_PTR_SUB, lhs, rhs, tok);
  if (lhs->ty->base && rhs->ty->base)
    return new_node_binary(ND_PTR_DIFF, lhs, rhs, tok);
  error_tok(tok, "invalid operands");
}

// Search variable name. Return NULL if not found.
Var *find_var(char *name)
{
  for (VarList *vl = locals; vl; vl = vl->next)
  {
    Var *var = vl->var;
    if (strlen(var->name) == strlen(name) && !memcmp(name, var->name, strlen(name)))
      return var;
  }

  for (VarList *vl = globals; vl; vl = vl->next)
  {
    Var *var = vl->var;
    if (strlen(var->name) == strlen(name) && !memcmp(name, var->name, strlen(name)))
      return var;
  }

  return NULL;
}

// Return new variable
static Var *new_var(char *name, Type *ty, bool is_local)
{
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->ty = ty;
  var->is_local = is_local;
  return var;
}

// Return new local variable
static Var *new_lvar(char *name, Type *ty)
{
  Var *var = new_var(name, ty, true);
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
  return var;
}

// Return new global variable
static Var *new_gvar(char *name, Type *ty)
{
  Var *var = new_var(name, ty, false);
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = globals;
  globals = vl;
  return var;
}

// Determine wheter the next top-level is a function
// or a global variable by looking ahead input tokens.
static bool is_function()
{
  Token *tok = token;
  basetype();
  bool isfunc = consume_ident() && consume("(");
  token = tok;
  return isfunc;
}

// program = (global-var | function)*
Program *program()
{
  Function head = {};
  Function *cur = &head;
  globals = NULL;

  while (!at_eof())
  {
    if (is_function())
    {
      cur->next = function();
      cur = cur->next;
    }
    else
    {
      global_var();
    }
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->fns = head.next;
  return prog;
}

static Type *basetype()
{
  expect("int");
  Type *ty = int_type;
  while (consume("*"))
    ty = pointer_to(ty);
  return ty;
}

static Type *read_type_suffix(Type *base)
{
  if (!consume("["))
    return base;
  int len = expect_number();
  expect("]");
  base = read_type_suffix(base);
  return array_of(base, len);
}

static VarList *read_func_param()
{
  Type *ty = basetype();
  char *name = expect_ident();
  ty = read_type_suffix(ty);

  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = new_lvar(name, ty);
  return vl;
}

static VarList *read_func_params()
{
  if (consume(")"))
    return NULL;

  VarList head = {};
  head.next = read_func_param();
  VarList *cur = head.next;
  while (consume(","))
  {
    cur->next = read_func_param();
    cur = cur->next;
  }
  expect(")");

  return head.next;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
// params = (param ",")*  param
// param = basetype ident
static Function *function()
{
  locals = NULL;

  Function *fn = calloc(1, sizeof(Function));
  basetype();
  fn->name = expect_ident();
  expect("(");
  fn->params = read_func_params();
  expect("{");

  Node head = {};
  Node *cur = &head;
  while (!consume("}"))
  {
    cur->next = stmt();
    cur = cur->next;
  }
  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

// global-var = basetype ident ("[" num "]")* ";"
static void global_var()
{
  Type *ty = basetype();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  expect(";");
  new_gvar(name, ty);
}

// declaration = basetype ident ("[" num "]")* ("=" expr)? ";"
static Node *declaration()
{
  Token *tok = token;
  Type *ty = basetype();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  Var *var = new_lvar(name, ty);

  if (consume(";"))
    return new_node(ND_NULL, tok);

  expect("=");
  Node *lhs = new_node_var(var, tok);
  Node *rhs = expr();
  expect(";");
  Node *node = new_node_binary(ND_ASSIGN, lhs, rhs, tok);
  return new_node_unary(ND_EXPR_STMT, node, tok);
}

static Node *read_expr_stmt()
{
  Token *tok = token;
  return new_node_unary(ND_EXPR_STMT, expr(), tok);
}

Node *stmt()
{
  Node *node = stmt2();
  add_type(node);
  return node;
}

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | declaration
//      | "return" expr ";"
static Node *stmt2()
{
  Token *tok;
  if (tok = consume("return"))
  {
    Node *node = new_node_unary(ND_RETURN, expr(), tok);
    expect(";");
    return node;
  }

  if (tok = consume("if"))
  {
    Node *node = new_node(ND_IF, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }

  if (tok = consume("while"))
  {
    Node *node = new_node(ND_WHILE, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (tok = consume("for"))
  {
    Node *node = new_node(ND_FOR, tok);
    expect("(");
    if (!consume(";"))
    {
      node->init = read_expr_stmt();
      expect(";");
    }
    // For "for (a; b; c) {...}" we regard b as true if b is empty.
    node->cond = new_node_num(1, tok);
    if (!consume(";"))
    {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")"))
    {
      node->inc = read_expr_stmt();
      expect(")");
    }
    node->then = stmt();
    return node;
  }

  if (tok = consume("{"))
  {
    Node *node = new_node(ND_BLOCK, tok);
    Node head = {};
    Node *cur = &head;
    for (;;)
    {
      if (!consume("}"))
      {
        cur->next = stmt();
        cur = cur->next;
      }
      else
      {
        node->block = head.next;
        return node;
      }
    }
  }

  if (tok = peek("int"))
    return declaration();

  Node *node = read_expr_stmt();
  expect(";");
  return node;
}

// expr = assign
Node *expr()
{
  return assign();
}

// assign = eauality ("=" assign)?
Node *assign()
{
  Node *node = equality();
  Token *tok;
  if (tok = consume("="))
    node = new_node_binary(ND_ASSIGN, node, assign(), tok);
  return node;
}

// eauality = relationam ("==" relationall | "!=" relational)*
static Node *equality()
{
  Node *node = relational();
  Token *tok;
  for (;;)
  {
    if (tok = consume("=="))
      node = new_node_binary(ND_EQ, node, relational(), tok);
    else if (tok = consume("!="))
      node = new_node_binary(ND_NE, node, relational(), tok);
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational()
{
  Node *node = add();
  Token *tok;

  for (;;)
  {
    if (tok = consume("<"))
      node = new_node_binary(ND_LT, node, add(), tok);
    else if (tok = consume("<="))
      node = new_node_binary(ND_LE, node, add(), tok);
    else if (tok = consume(">"))
      node = new_node_binary(ND_LT, add(), node, tok);
    else if (tok = consume(">="))
      node = new_node_binary(ND_LE, add(), node, tok);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add()
{
  Node *node = mul();
  Token *tok;

  for (;;)
  {
    if (tok = consume("+"))
      node = new_node_add(node, mul(), tok);
    else if (tok = consume("-"))
      node = new_node_sub(node, mul(), tok);
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul()
{
  Node *node = unary();
  Token *tok;

  for (;;)
  {
    if (tok = consume("*"))
      node = new_node_binary(ND_MUL, node, unary(), tok);
    else if (tok = consume("/"))
      node = new_node_binary(ND_DIV, node, unary(), tok);
    else
      return node;
  }
}

// unary = ("sizeof" | "+" | "-" | "*" | "&")? unary
//       | postfix
static Node *unary()
{
  Token *tok;
  if (tok = consume("sizeof"))
    return new_node_unary(ND_SIZEOF, unary(), tok);
  if (tok = consume("+"))
    return unary();
  if (tok = consume("-"))
    return new_node_binary(ND_SUB, new_node_num(0, tok), unary(), tok);
  if (tok = consume("*"))
    return new_node_unary(ND_DEREF, unary(), tok);
  if (tok = consume("&"))
    return new_node_unary(ND_ADDR, unary(), tok);
  return postfix();
}

// postfix = primary ("[" expr "]")*
static Node *postfix()
{
  Node *node = primary();
  Token *tok;

  while (tok = consume("["))
  {
    // x[y] is short for *(x + y)
    Node *exp = new_node_add(node, expr(), tok);
    expect("]");
    node = new_node_unary(ND_DEREF, exp, tok);
  }
  return node;
}

// primary = num
//         | ident ( "(" args? ")" )?
//         | "(" expr ")"
// args = (expr ",")*  expr
static Node *primary()
{
  Token *tok;
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  if (tok = consume_ident())
  {
    char *identname = strndup(tok->loc, tok->len);
    // Function call
    if (consume("("))
    {
      Node *node = new_node(ND_FUNCALL, tok);
      node->funcname = identname;
      if (!consume(")"))
      {
        // Function call arguments
        Node head = {};
        Node *cur = &head;
        cur->next = expr();
        cur = cur->next;
        while (consume(","))
        {
          cur->next = expr();
          cur = cur->next;
        }
        node->arg = head.next;
        expect(")");
      }
      return node;
    }

    // Local variable
    Var *var = find_var(identname);
    if (!var)
      error_tok(tok, "undefind variable");
    return new_node_var(var, tok);
  }

  tok = token;
  return new_node_num(expect_number(), tok);
}
