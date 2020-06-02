#include "9cc.h"

/*********************************************
* ...parser...
*********************************************/

// Local variables
static VarList *locals;
static VarList *globals;

static Function *function(Token **rest, Token *tok);
static Type *basetype(Token **rest, Token *tok);
static void global_var(Token **rest, Token *tok);

static Node *declaration(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *stmt2(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *postfix(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

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

static char *new_gvar_name()
{
  static int cnt = 0;
  char *buf = malloc(20);
  sprintf(buf, ".L.data.%d", cnt++);
  return buf;
}

static Var *new_string_literal(char *p, int len)
{
  Type *ty = array_of(char_type, len);
  Var *var = new_gvar(new_gvar_name(), ty);
  var->init_data = p;
  return var;
}

static char *get_ident(Token *tok)
{
  if (tok->kind != TK_IDENT)
    error_tok(tok, "expected an identifier");
  return strndup(tok->loc, tok->len);
}

static int get_number(Token *tok)
{
  if (tok->kind != TK_NUM)
    error_tok(tok, "expected a number");
  return tok->val;
}

// Determine wheter the next top-level is a function
// or a global variable by looking ahead input tokens.
static bool is_function(Token *tok)
{
  basetype(&tok, tok);
  bool isfunc = get_ident(tok) && equal(tok->next, "(");
  return isfunc;
}

// program = (global-var | function)*
Program *parse(Token *tok)
{
  Function head = {};
  Function *cur = &head;
  globals = NULL;

  while (tok->kind != TK_EOF)
  {
    if (is_function(tok))
    {
      cur->next = function(&tok, tok);
      cur = cur->next;
    }
    else
    {
      global_var(&tok, tok);
    }
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->fns = head.next;
  return prog;
}

static Type *basetype(Token **rest, Token *tok)
{
  Type *ty;
  if (equal(tok, "char"))
  {
    tok = tok->next;
    ty = char_type;
  }
  else if (equal(tok, "int"))
  {
    tok = tok->next;
    ty = int_type;
  }
  else
  {
    error_tok(tok, "expected type name");
  }

  while (equal(tok, "*"))
  {
    tok = tok->next;
    ty = pointer_to(ty);
  }
  *rest = tok;
  return ty;
}

// Returns true if a givin token represents a type
static bool is_typename(Token *tok)
{
  return equal(tok, "char") || equal(tok, "int");
}

static Type *read_type_suffix(Type *base, Token **rest, Token *tok)
{
  if (!equal(tok, "["))
  {
    *rest = tok;
    return base;
  }
  tok = skip(tok, "[");
  int len = get_number(tok);
  tok = skip(tok->next, "]");
  base = read_type_suffix(base, &tok, tok);
  *rest = tok;
  return array_of(base, len);
}

static VarList *read_func_param(Token **rest, Token *tok)
{
  Type *ty = basetype(&tok, tok);
  char *name = get_ident(tok);
  ty = read_type_suffix(ty, &tok, tok->next);
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = new_lvar(name, ty);
  *rest = tok;
  return vl;
}

static VarList *read_func_params(Token **rest, Token *tok)
{
  if (equal(tok, ")"))
  {
    *rest = tok->next;
    return NULL;
  }

  VarList head = {};
  head.next = read_func_param(&tok, tok);
  VarList *cur = head.next;
  while (equal(tok, ","))
  {
    cur->next = read_func_param(&tok, tok->next);
    cur = cur->next;
  }
  tok = skip(tok, ")");

  *rest = tok;
  return head.next;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
// params = (param ",")*  param
// param = basetype ident
static Function *function(Token **rest, Token *tok)
{
  locals = NULL;

  Function *fn = calloc(1, sizeof(Function));
  basetype(&tok, tok);
  fn->name = get_ident(tok);
  tok = skip(tok->next, "(");
  fn->params = read_func_params(&tok, tok);
  tok = skip(tok, "{");

  Node head = {};
  Node *cur = &head;
  while (!equal(tok, "}"))
  {
    cur->next = stmt(&tok, tok);
    cur = cur->next;
  }
  fn->node = head.next;
  fn->locals = locals;
  *rest = tok->next;
  return fn;
}

// global-var = basetype ident ("[" num "]")* ";"
static void global_var(Token **rest, Token *tok)
{
  Type *ty = basetype(&tok, tok);
  char *name = get_ident(tok);
  ty = read_type_suffix(ty, &tok, tok->next);
  *rest = skip(tok, ";");
  new_gvar(name, ty);
}

// declaration = basetype ident ("[" num "]")* ("=" expr)? ";"
static Node *declaration(Token **rest, Token *tok)
{
  Type *ty = basetype(&tok, tok);
  char *name = get_ident(tok);
  ty = read_type_suffix(ty, &tok, tok->next);
  Var *var = new_lvar(name, ty);
  if (equal(tok, ";"))
  {
    *rest = tok->next;
    return new_node(ND_NULL, tok);
  }

  tok = skip(tok, "=");
  Node *lhs = new_node_var(var, tok);
  Node *rhs = expr(&tok, tok);
  *rest = skip(tok, ";");
  Node *node = new_node_binary(ND_ASSIGN, lhs, rhs, tok);
  return new_node_unary(ND_EXPR_STMT, node, tok);
}

// expr-stmt = expr
static Node *expr_stmt(Token **rest, Token *tok)
{
  Node *node = new_node(ND_EXPR_STMT, tok);
  node->lhs = expr(rest, tok);
  return node;
}

Node *stmt(Token **rest, Token *tok)
{
  Node *node = stmt2(rest, tok);
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
static Node *stmt2(Token **rest, Token *tok)
{
  if (equal(tok, "return"))
  {
    Node *node = new_node(ND_RETURN, tok);
    node->lhs = expr(&tok, tok->next);
    *rest = skip(tok, ";");
    return node;
  }

  if (equal(tok, "if"))
  {
    Node *node = new_node(ND_IF, tok);
    tok = skip(tok->next, "(");
    node->cond = expr(&tok, tok);
    tok = skip(tok, ")");
    node->then = stmt(&tok, tok);
    if (equal(tok, "else"))
      node->els = stmt(&tok, tok->next);
    *rest = tok;
    return node;
  }

  if (equal(tok, "while"))
  {
    Node *node = new_node(ND_WHILE, tok);
    tok = skip(tok->next, "(");
    node->cond = expr(&tok, tok);
    tok = skip(tok, ")");

    node->then = stmt(&tok, tok);
    *rest = tok;
    return node;
  }

  if (equal(tok, "for"))
  {
    Node *node = new_node(ND_FOR, tok);
    tok = skip(tok->next, "(");

    if (!equal(tok, ";"))
      node->init = expr_stmt(&tok, tok);
    tok = skip(tok, ";");

    // For "for (a; b; c) {...}" we regard b as true if b is empty.
    node->cond = new_node_num(1, tok);
    if (!equal(tok, ";"))
      node->cond = expr(&tok, tok);
    tok = skip(tok, ";");

    if (!equal(tok, ")"))
      node->inc = expr_stmt(&tok, tok);
    tok = skip(tok, ")");

    node->then = stmt(&tok, tok);

    *rest = tok;
    return node;
  }

  if (equal(tok, "{"))
  {
    Node *node = new_node(ND_BLOCK, tok);
    Node head = {};
    Node *cur = &head;
    tok = skip(tok, "{");
    for (;;)
    {
      if (!equal(tok, "}"))
      {
        cur->next = stmt(&tok, tok);
        cur = cur->next;
      }
      else
      {
        tok = skip(tok, "}");
        node->body = head.next;
        *rest = tok;
        return node;
      }
    }
  }

  if (is_typename(tok))
    return declaration(rest, tok);

  Node *node = expr_stmt(&tok, tok);
  *rest = skip(tok, ";");
  return node;
}

// expr = assign
static Node *expr(Token **rest, Token *tok)
{
  return assign(rest, tok);
}

// assign = eauality ("=" assign)?
static Node *assign(Token **rest, Token *tok)
{
  Node *node = equality(&tok, tok);
  if (equal(tok, "="))
  {
    Node *rhs = assign(&tok, tok->next);
    node = new_node_binary(ND_ASSIGN, node, rhs, tok);
  }
  *rest = tok;
  return node;
}

// eauality = relationam ("==" relationall | "!=" relational)*
static Node *equality(Token **rest, Token *tok)
{
  Node *node = relational(&tok, tok);

  for (;;)
  {
    if (equal(tok, "=="))
    {
      Node *rhs = relational(&tok, tok->next);
      node = new_node_binary(ND_EQ, node, rhs, tok);
      continue;
    }
    else if (equal(tok, "!="))
    {
      Node *rhs = relational(&tok, tok->next);
      node = new_node_binary(ND_NE, node, rhs, tok);
      continue;
    }
    *rest = tok;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *tok)
{
  Node *node = add(&tok, tok);

  for (;;)
  {
    if (equal(tok, "<"))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_node_binary(ND_LT, node, rhs, tok);
      continue;
    }

    if (equal(tok, "<="))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_node_binary(ND_LE, node, rhs, tok);
      continue;
    }

    if (equal(tok, ">"))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_node_binary(ND_LT, rhs, node, tok);
      continue;
    }

    if (equal(tok, ">="))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_node_binary(ND_LE, rhs, node, tok);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *tok)
{
  Node *node = mul(&tok, tok);

  for (;;)
  {
    if (equal(tok, "+"))
    {
      Node *rhs = mul(&tok, tok->next);
      // node = new_node_binary(ND_ADD, node, rhs, tok);
      node = new_node_add(node, rhs, tok);
      continue;
    }

    if (equal(tok, "-"))
    {
      Node *rhs = mul(&tok, tok->next);
      // node = new_node_binary(ND_SUB, node, rhs, tok);
      node = new_node_sub(node, rhs, tok);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **rest, Token *tok)
{
  Node *node = unary(&tok, tok);

  for (;;)
  {
    if (equal(tok, "*"))
    {
      Node *rhs = unary(&tok, tok->next);
      node = new_node_binary(ND_MUL, node, rhs, tok);
      continue;
    }

    if (equal(tok, "/"))
    {
      Node *rhs = unary(&tok, tok->next);
      node = new_node_binary(ND_DIV, node, rhs, tok);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// unary = ("sizeof" | "+" | "-" | "*" | "&")? unary
//       | postfix
static Node *unary(Token **rest, Token *tok)
{
  if (equal(tok, "sizeof"))
    return new_node_unary(ND_SIZEOF, unary(rest, tok->next), tok);
  if (equal(tok, "+"))
    return unary(rest, tok->next);
  if (equal(tok, "-"))
    return new_node_binary(ND_SUB, new_node_num(0, tok), unary(rest, tok->next), tok);
  if (equal(tok, "*"))
    return new_node_unary(ND_DEREF, unary(rest, tok->next), tok);
  if (equal(tok, "&"))
    return new_node_unary(ND_ADDR, unary(rest, tok->next), tok);
  return postfix(rest, tok);
}

// postfix = primary ("[" expr "]")*
static Node *postfix(Token **rest, Token *tok)
{
  Node *node = primary(&tok, tok);

  while (equal(tok, "["))
  {
    // x[y] is short for *(x + y)
    Token *start = tok;
    Node *idx = expr(&tok, tok->next);
    tok = skip(tok, "]");
    node = new_node_unary(ND_DEREF, new_node_add(node, idx, start), start);
  }
  *rest = tok;
  return node;
}

// primary = "(" "{" stmt stmt* "}" ")"
//         | "(" expr ")"
//         | num
//         | str
//         | ident ( "(" args? ")" )?
// args = (expr ",")*  expr
static Node *primary(Token **rest, Token *tok)
{
  if (equal(tok, "(") && equal(tok->next, "{"))
  {
    // This is a GNU statement expression.
    Node *node = new_node(ND_STMT_EXPR, tok);
    Node head = {};
    Node *cur = &head;
    tok = skip(tok->next, "{");
    for (;;)
    {
      if (!equal(tok, "}"))
      {
        cur->next = stmt(&tok, tok);
        cur = cur->next;
      }
      else
      {
        break;
      }
    }
    tok = skip(tok, "}");
    *rest = skip(tok, ")");
    node->body = head.next;

    cur = node->body;
    while (cur->next)
      cur = cur->next;

    if (cur->kind != ND_EXPR_STMT)
      error_tok(cur->tok, "statement expression returning void is not supported");
    return node;
  }

  if (equal(tok, "("))
  {
    Node *node = expr(&tok, tok->next);
    *rest = skip(tok, ")");
    return node;
  }

  if (tok->kind == TK_IDENT)
  {
    char *identname = strndup(tok->loc, tok->len);
    // Function call
    if (equal(tok->next, "("))
    {
      Node *node = new_node(ND_FUNCALL, tok);
      node->funcname = identname;
      tok = tok->next;
      if (!equal(tok->next, ")"))
      {
        // Function call arguments
        Node head = {};
        Node *cur = &head;
        cur->next = expr(&tok, tok->next);
        cur = cur->next;
        while (equal(tok, ","))
        {
          cur->next = expr(&tok, tok->next);
          cur = cur->next;
        }
        node->arg = head.next;
        tok = skip(tok, ")");
      }
      else
      {
        tok = skip(tok->next, ")");
      }
      *rest = tok;
      return node;
    }

    // Local variable
    Var *var = find_var(identname);
    if (!var)
      error_tok(tok, "undefind variable");
    *rest = tok->next;
    return new_node_var(var, tok);
  }

  // String literal
  if (tok->kind == TK_STR)
  {
    Var *var = new_string_literal(tok->contents, tok->cont_len);
    *rest = tok->next;
    return new_node_var(var, tok);
  }

  // Numeric literal
  if (tok->kind != TK_NUM)
    error_tok(tok, "expected expression");

  *rest = tok->next;
  return new_node_num(get_number(tok), tok);
}