#include "9cc.h"

/*********************************************
* ...parser...
*********************************************/

// Scope for local or global variables.
typedef struct VarScope VarScope;
struct VarScope
{
  VarScope *next;
  char *name;
  int depth;
  Var *var;
};

// Local variables
static VarList *locals;
// Global variables
static VarList *globals;

static VarScope *var_scope;

// scope_depth is incremented by one at "{" and decremented
// by one at "}"
static int scope_depth;

static Type *typespec(Token **rest, Token *tok);
static Type *declarator(Token **rest, Token *tok, Type *ty);
static Function *funcdef(Token **rest, Token *tok);
static Node *declaration(Token **rest, Token *tok);
static Node *compound_stmt(Token **Rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Type *struct_decl(Token **rest, Token *tok);
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

static Node *new_unary(NodeKind kind, Node *expr, Token *tok)
{
  Node *node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_add(Node *lhs, Node *rhs, Token *tok)
{
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_ADD, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_binary(ND_PTR_ADD, lhs, rhs, tok);
  if (is_integer(lhs->ty) && rhs->ty->base)
    return new_binary(ND_PTR_ADD, lhs, rhs, tok);
  error_tok(tok, "invalid operands");
}

static Node *new_sub(Node *lhs, Node *rhs, Token *tok)
{
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_SUB, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_binary(ND_PTR_SUB, lhs, rhs, tok);
  if (lhs->ty->base && rhs->ty->base)
    return new_binary(ND_PTR_DIFF, lhs, rhs, tok);
  error_tok(tok, "invalid operands");
}

static void enter_scope()
{
  scope_depth++;
}

static void leave_scope()
{
  scope_depth--;
  while (var_scope && var_scope->depth > scope_depth)
    var_scope = var_scope->next;
}

// Search variable by name. Return NULL if not found.
Var *find_var(Token *tok)
{
  for (VarScope *sc = var_scope; sc; sc = sc->next)
  {
    if (strlen(sc->name) == tok->len && !strncmp(tok->loc, sc->name, tok->len))
      return sc->var;
  }
  return NULL;
}

static VarScope *push_scope(char *name, Var *var)
{
  VarScope *sc = calloc(1, sizeof(VarScope));
  sc->next = var_scope;
  sc->name = name;
  sc->var = var;
  sc->depth = scope_depth;
  var_scope = sc;
  return sc;
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
  push_scope(name, var);
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
  push_scope(name, var);
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

// program = (global-var | funcdef)*
Program *parse(Token *tok)
{
  Function head = {};
  Function *cur = &head;
  globals = NULL;

  while (tok->kind != TK_EOF)
  {
    Token *start = tok;
    Type *basety = typespec(&tok, tok);
    Type *ty = declarator(&tok, tok, basety);

    // Function
    if (ty->kind == TY_FUNC)
    {
      cur = cur->next = funcdef(&tok, start);
      continue;
    }

    // Global variable
    for (;;)
    {
      new_gvar(get_ident(ty->name), ty);
      if (equal(tok, ";"))
      {
        tok = tok->next;
        break;
      }
      tok = skip(tok, ",");
      ty = declarator(&tok, tok, basety);
    }
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->fns = head.next;
  return prog;
}

// Returns true if a givin token represents a type
static bool is_typename(Token *tok)
{
  return equal(tok, "char") || equal(tok, "int") || equal(tok, "struct");
}

// funcdef = typespec declarator compound-stmt
static Function *funcdef(Token **rest, Token *tok)
{
  locals = NULL;

  Type *ty = typespec(&tok, tok);
  ty = declarator(&tok, tok, ty);

  Function *fn = calloc(1, sizeof(Function));
  fn->name = get_ident(ty->name);

  enter_scope();
  for (Type *t = ty->params; t; t = t->next)
    new_lvar(get_ident(t->name), t);
  fn->params = locals;

  tok = skip(tok, "{");
  fn->node = compound_stmt(rest, tok)->body;
  fn->locals = locals;
  leave_scope();
  return fn;
}

// typespec = "char" | "int" | struct-decl
static Type *typespec(Token **rest, Token *tok)
{
  if (equal(tok, "char"))
  {
    *rest = tok->next;
    return char_type;
  }
  else if (equal(tok, "int"))
  {
    *rest = tok->next;
    return int_type;
  }
  else if (equal(tok, "struct"))
    return struct_decl(rest, tok->next);
  else
  {
    error_tok(tok, "expected type name");
  }
}

// func-params = (param ("," param)*)? ")"
// param       = typespec declarator
static Type *func_params(Token **rest, Token *tok, Type *ty)
{
  Type head = {};
  Type *cur = &head;

  while (!equal(tok, ")"))
  {
    if (cur != &head)
      tok = skip(tok, ",");
    Type *basety = typespec(&tok, tok);
    Type *ty = declarator(&tok, tok, basety);
    cur = cur->next = copy_type(ty);
  }

  ty = func_type(ty);
  ty->params = head.next;
  *rest = tok->next;
  return ty;
}

// type-suffix = "(" func-params
//             | "[" num "]" type-suffix
//             | ε
static Type *type_suffix(Token **rest, Token *tok, Type *ty)
{
  if (equal(tok, "("))
    return func_params(rest, tok->next, ty);

  if (equal(tok, "["))
  {
    int len = get_number(tok->next);
    tok = skip(tok->next->next, "]");
    ty = type_suffix(rest, tok, ty);
    return array_of(ty, len);
  }

  *rest = tok;
  return ty;
}

// declarator = "*"* ident type-suffix
static Type *declarator(Token **rest, Token *tok, Type *ty)
{
  while (equal(tok, "*"))
  {
    tok = tok->next;
    ty = pointer_to(ty);
  }

  if (tok->kind != TK_IDENT)
    error_tok(tok, "expected a variable name");
  ty = type_suffix(rest, tok->next, ty);
  ty->name = tok;
  return ty;
}

// declaration = typespec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
static Node *declaration(Token **rest, Token *tok)
{
  Type *basety = typespec(&tok, tok);

  Node head = {};
  Node *cur = &head;
  int cnt = 0;

  while (!equal(tok, ";"))
  {
    if (cnt++ > 0)
      tok = skip(tok, ",");

    Type *ty = declarator(&tok, tok, basety);
    Var *var = new_lvar(get_ident(ty->name), ty);

    if (!equal(tok, "="))
      continue;

    Node *lhs = new_node_var(var, ty->name);
    Node *rhs = assign(&tok, tok->next);
    Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
    cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);
  }

  Node *node = new_node(ND_BLOCK, tok);
  node->body = head.next;
  *rest = tok->next;
  return node;
}

// compound-stmt = (declaration | stmt)* "}"
static Node *compound_stmt(Token **rest, Token *tok)
{
  Node *node = new_node(ND_BLOCK, tok);
  Node head = {};
  Node *cur = &head;

  enter_scope();

  while (!equal(tok, "}"))
  {
    if (is_typename(tok))
      cur = cur->next = declaration(&tok, tok);
    else
      cur = cur->next = stmt(&tok, tok);
    add_type(cur);
  }

  leave_scope();

  node->body = head.next;
  *rest = tok->next;
  return node;
}

// expr-stmt = expr
static Node *expr_stmt(Token **rest, Token *tok)
{
  Node *node = new_node(ND_EXPR_STMT, tok);
  node->lhs = expr(rest, tok);
  return node;
}

// stmt = expr ";"
//      | "{" compound-stmt
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
static Node *stmt(Token **rest, Token *tok)
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
    return compound_stmt(rest, tok->next);

  Node *node = expr_stmt(&tok, tok);
  *rest = skip(tok, ";");
  return node;
}

// expr = assign ("," expr)?
static Node *expr(Token **rest, Token *tok)
{
  Node *node = assign(&tok, tok);

  if (equal(tok, ","))
    return new_binary(ND_COMMA, node, expr(rest, tok->next), tok);

  *rest = tok;
  return node;
}

// assign = eauality ("=" assign)?
static Node *assign(Token **rest, Token *tok)
{
  Node *node = equality(&tok, tok);
  if (equal(tok, "="))
  {
    Node *rhs = assign(&tok, tok->next);
    node = new_binary(ND_ASSIGN, node, rhs, tok);
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
      node = new_binary(ND_EQ, node, rhs, tok);
      continue;
    }
    else if (equal(tok, "!="))
    {
      Node *rhs = relational(&tok, tok->next);
      node = new_binary(ND_NE, node, rhs, tok);
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
      node = new_binary(ND_LT, node, rhs, tok);
      continue;
    }

    if (equal(tok, "<="))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_binary(ND_LE, node, rhs, tok);
      continue;
    }

    if (equal(tok, ">"))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_binary(ND_LT, rhs, node, tok);
      continue;
    }

    if (equal(tok, ">="))
    {
      Node *rhs = add(&tok, tok->next);
      node = new_binary(ND_LE, rhs, node, tok);
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
      node = new_add(node, rhs, tok);
      continue;
    }

    if (equal(tok, "-"))
    {
      Node *rhs = mul(&tok, tok->next);
      // node = new_node_binary(ND_SUB, node, rhs, tok);
      node = new_sub(node, rhs, tok);
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
      node = new_binary(ND_MUL, node, rhs, tok);
      continue;
    }

    if (equal(tok, "/"))
    {
      Node *rhs = unary(&tok, tok->next);
      node = new_binary(ND_DIV, node, rhs, tok);
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
    return new_unary(ND_SIZEOF, unary(rest, tok->next), tok);
  if (equal(tok, "+"))
    return unary(rest, tok->next);
  if (equal(tok, "-"))
    return new_binary(ND_SUB, new_node_num(0, tok), unary(rest, tok->next), tok);
  if (equal(tok, "*"))
    return new_unary(ND_DEREF, unary(rest, tok->next), tok);
  if (equal(tok, "&"))
    return new_unary(ND_ADDR, unary(rest, tok->next), tok);
  return postfix(rest, tok);
}

// struct-members = (typespec declarator ("," declarator)* ";")*
static Member *struct_members(Token **rest, Token *tok)
{
  Member head = {};
  Member *cur = &head;

  while (!equal(tok, "}"))
  {
    Type *basety = typespec(&tok, tok);
    int cnt = 0;

    while (!equal(tok, ";"))
    {
      if (cnt++)
        tok = skip(tok, ",");

      Member *mem = calloc(1, sizeof(Member));
      mem->ty = declarator(&tok, tok, basety);
      mem->name = mem->ty->name;
      cur = cur->next = mem;
    }
    tok = tok->next;
  }

  *rest = tok->next;
  return head.next;
}

// sturct-decl = "{" struct-members
static Type *struct_decl(Token **rest, Token *tok)
{
  tok = skip(tok, "{");

  // Construct a struct object.
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_STRUCT;
  ty->members = struct_members(rest, tok);

  // Assign offsets within the struct to members.
  int offset = 0;
  for (Member *mem = ty->members; mem; mem = mem->next)
  {
    offset = align_to(offset, mem->ty->align);
    mem->offset = offset;
    offset += mem->ty->size;

    if (ty->align < mem->ty->align)
      ty->align = mem->ty->align;
  }
  ty->size = align_to(offset, ty->align);

  return ty;
}

static Member *get_struct_member(Type *ty, Token *tok)
{
  for (Member *mem = ty->members; mem; mem = mem->next)
    if (mem->name->len == tok->len &&
        !strncmp(mem->name->loc, tok->loc, tok->len))
      return mem;
  error_tok(tok, "no such member");
}

// reference to a struct member
// object.member
// ---> lhs->tok->loc: object...., tok->loc: member...
static Node *struct_ref(Node *lhs, Token *tok)
{
  add_type(lhs);
  if (lhs->ty->kind != TY_STRUCT)
    error_tok(lhs->tok, "not a struct");

  Node *node = new_unary(ND_MEMBER, lhs, tok);
  node->member = get_struct_member(lhs->ty, tok);
  return node;
}

// postfix = primary ("[" expr "]" | "." ident)*
static Node *postfix(Token **rest, Token *tok)
{
  Node *node = primary(&tok, tok);

  for (;;)
  {
    if (equal(tok, "["))
    {
      // x[y] is short for *(x + y)
      Token *start = tok;
      Node *idx = expr(&tok, tok->next);
      tok = skip(tok, "]");
      node = new_unary(ND_DEREF, new_add(node, idx, start), start);
      continue;
    }

    if (equal(tok, "."))
    {
      node = struct_ref(node, tok->next);
      tok = tok->next->next;
      continue;
    }

    *rest = tok;
    return node;
  }
}

// funcall = ident "(" (assign ("," assign)*)? ")"
//
// foo(a, b, c) is compiled to ({t1=a; t2=b; t3=c; foo(t1, t2, t3); })
// where t1, t2 and t3 are fresh local variables.
static Node *funcall(Token **rest, Token *tok)
{
  Token *start = tok;
  tok = tok->next->next;

  Node *node = new_node(ND_NULL_EXPR, tok);
  Var **args = NULL;
  int nargs = 0;

  while (!equal(tok, ")"))
  {
    if (nargs)
      tok = skip(tok, ",");

    Node *arg = assign(&tok, tok);
    add_type(arg);

    Var *var = arg->ty->base ? new_lvar("", pointer_to(arg->ty->base))
                             : new_lvar("", arg->ty);
    args = realloc(args, sizeof(*args) * (nargs + 1));
    args[nargs] = var;
    nargs++;

    Node *expr = new_binary(ND_ASSIGN, new_node_var(var, tok), arg, tok);
    node = new_binary(ND_COMMA, node, expr, tok);
  }

  *rest = skip(tok, ")");

  Node *funcall = new_node(ND_FUNCALL, start);
  funcall->funcname = strndup(start->loc, start->len);
  funcall->args = args;
  funcall->nargs = nargs;
  return new_binary(ND_COMMA, node, funcall, tok);
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
    node->body = compound_stmt(&tok, tok->next->next)->body;
    *rest = skip(tok, ")");

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
    // Function call
    if (equal(tok->next, "("))
      return funcall(rest, tok);

    // Local variable
    Var *var = find_var(tok);
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