#include "9cc.h"

/*********************************************
* ...type...
*********************************************/

Type *int_type = &(Type){TY_INT, NULL, 8};

bool is_integer(Type *ty)
{
  return ty->kind == TY_INT;
}

Type *pointer_to(Type *base)
{
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->base = base;
  ty->size = 8;
  return ty;
}

Type *array_of(Type *base, int len)
{
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARR;
  ty->base = base;
  ty->size = base->size * len;
  ty->array_len = len;
  return ty;
}

void add_type(Node *node)
{
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->block; n; n = n->next)
    add_type(n);
  for (Node *n = node->arg; n; n = n->next)
    add_type(n);

  switch (node->kind)
  {
  case ND_ADD:
  case ND_SUB:
  case ND_PTR_DIFF:
  case ND_MUL:
  case ND_DIV:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_FUNCALL:
  case ND_NUM:
    node->ty = int_type;
    return;
  case ND_PTR_ADD:
  case ND_PTR_SUB:
  case ND_ASSIGN:
    node->ty = node->lhs->ty;
    return;
  case ND_LVAR:
    node->ty = node->lvar->ty;
    return;
  case ND_ADDR:
    if (node->lhs->ty->kind == TY_ARR)
      node->ty = pointer_to(node->lhs->ty->base);
    else
      node->ty = pointer_to(node->lhs->ty);
    return;
  case ND_DEREF:
    if (node->lhs->ty->base)
      node->ty = node->lhs->ty->base;
    else
      error_tok(node->tok, "invalid pointer dereference");
    return;
  case ND_SIZEOF:
    node->ty = int_type;
  }
}