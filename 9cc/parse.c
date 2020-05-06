#include "9cc.h"

/*********************************************
* ...parser...
*********************************************/

// Local variables
static VarList *locals;

static Function *function();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
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

// Search variable name. Return NULL if not found.
LVar *find_lvar(char *name)
{
  for (VarList *var = locals; var; var = var->next)
  {
    LVar *lvar = var->lvar;
    if (lvar->len == strlen(name) && !memcmp(name, lvar->name, lvar->len))
      return lvar;
  }
  return NULL;
}

static LVar *new_lvar(char *name)
{
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->name = name;
  lvar->len = strlen(name);
  VarList *vl = calloc(1, sizeof(VarList));
  vl->lvar = lvar;
  vl->next = locals;
  locals = vl;
  return lvar;
}

// program = function*
Function *program()
{
  Function head = {};
  Function *cur = &head;
  while (!at_eof())
  {
    cur->next = function();
    cur = cur->next;
  }

  return head.next;
}

static VarList *read_func_params()
{
  if (consume(")"))
    return NULL;

  VarList head = {};
  head.next = calloc(1, sizeof(VarList));
  head.next->lvar = new_lvar(expect_ident());
  VarList *cur = head.next;
  while (consume(","))
  {
    cur->next = calloc(1, sizeof(VarList));
    cur->next->lvar = new_lvar(expect_ident());
    cur = cur->next;
  }
  expect(")");

  return head.next;
}

// function = ident "(" params? ")" "{" stmt* "}"
// params = (expr ",")*  expr
static Function *function()
{
  locals = NULL;

  Function *fn = calloc(1, sizeof(Function));
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

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
Node *stmt()
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
      node->init = expr();
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
      node->inc = expr();
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

  Node *node = expr();
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
      node = new_node_binary(ND_ADD, node, mul(), tok);
    else if (tok = consume("-"))
      node = new_node_binary(ND_SUB, node, mul(), tok);
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

// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
static Node *unary()
{
  Token *tok;

  if (tok = consume("+"))
    return primary();
  if (tok = consume("-"))
    return new_node_binary(ND_SUB, new_node_num(0, tok), primary(), tok);
  if (tok = consume("*"))
    return new_node_unary(ND_DEREF, unary(), tok);
  if (tok = consume("&"))
    return new_node_unary(ND_ADDR, unary(), tok);
  return primary();
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
    char *identname = strndup(tok->str, tok->len);
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
    Node *node = new_node(ND_LVAR, tok);
    LVar *lvar = find_lvar(identname);
    if (!lvar)
      lvar = new_lvar(identname);
    node->lvar = lvar;
    return node;
  }

  tok = token;
  return new_node_num(expect_number(), tok);
}
