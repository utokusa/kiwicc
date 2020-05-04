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

static Node *new_node(NodeKind kind)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

static Node *new_node_unary(NodeKind kind, Node *expr)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = expr;
  return node;
}

static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
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
  if (locals)
    lvar->offset = locals->lvar->offset + 8;
  else
    lvar->offset = 8;
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
  if (locals)
    fn->stack_size = locals->lvar->offset;
  else
    fn->stack_size = 0;
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
  if (consume("return"))
  {
    Node *node = new_node_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }

  if (consume("if"))
  {
    Node *node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }

  if (consume("while"))
  {
    Node *node = new_node(ND_WHILE);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (consume("for"))
  {
    Node *node = new_node(ND_FOR);
    expect("(");
    if (!consume(";"))
    {
      node->init = expr();
      expect(";");
    }
    // For "for (a; b; c) {...}" we regard b as true if b is empty.
    node->cond = new_node_num(1);
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

  if (consume("{"))
  {
    Node *node = new_node(ND_BLOCK);
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
  if (consume("="))
    node = new_node_binary(ND_ASSIGN, node, assign());
  return node;
}

// eauality = relationam ("==" relationall | "!=" relational)*
static Node *equality()
{
  Node *node = relational();

  for (;;)
  {
    if (consume("=="))
      node = new_node_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational()
{
  Node *node = add();

  for (;;)
  {
    if (consume("<"))
      node = new_node_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_node_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_node_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add()
{
  Node *node = mul();

  for (;;)
  {
    if (consume("+"))
      node = new_node_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul()
{
  Node *node = unary();

  for (;;)
  {
    if (consume("*"))
      node = new_node_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? primary
static Node *unary()
{
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node_binary(ND_SUB, new_node_num(0), primary());
  return primary();
}

// primary = num
//         | ident ( "(" args? ")" )?
//         | "(" expr ")"
// args = (expr ",")*  expr
static Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  char *identname = consume_ident();
  if (identname)
  {
    // Function call
    if (consume("("))
    {
      Node *node = new_node(ND_FUNCALL);
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
    Node *node = new_node(ND_LVAR);
    LVar *lvar = find_lvar(identname);
    if (!lvar)
      lvar = new_lvar(identname);
    node->offset = lvar->offset;
    return node;
  }

  return new_node_num(expect_number());
}
