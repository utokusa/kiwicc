#include "9cc.h"

/*********************************************
* ...parser...
*********************************************/

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

// program = stmt*
void program()
{
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

// stmt = expr ";"
//      | "if" "(" expr ")" stmt ("else if" "(" expr ")" stmt)* ("else" stmt)?
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

// primary = num | ident | "(" expr ")"
static Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok)
  {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar)
    {
      node->offset = lvar->offset;
    }
    else
    {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals)
        lvar->offset = locals->offset + 8;
      else
        lvar->offset = 0;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  return new_node_num(expect_number());
}
