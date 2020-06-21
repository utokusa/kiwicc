#include "9cc.h"

/*********************************************
* ...type...
*********************************************/

Type *char_type = &(Type){TY_CHAR, 1, 1};
Type *int_type = &(Type){TY_INT, 8, 8};

static Type *new_type(TypeKind kind, int size, int align)
{
  Type *ty = malloc(sizeof(Type));
  ty->kind = kind;
  ty->size = size;
  ty->align = align;
  return ty;
}

bool is_integer(Type *ty)
{
  return ty->kind == TY_CHAR || ty->kind == TY_INT;
}

Type *copy_type(Type *ty)
{
  Type *ret = malloc(sizeof(Type));
  *ret = *ty;
  return ret;
}

int align_to(int n, int align)
{
  return (n + align - 1) & ~(align - 1);
}

Type *pointer_to(Type *base)
{
  Type *ty = new_type(TY_PTR, 8, 8);
  ty->base = base;
  return ty;
}

Type *func_type(Type *return_ty)
{
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_FUNC;
  ty->return_ty = return_ty;
  return ty;
}

Type *array_of(Type *base, int len)
{
  Type *ty = new_type(TY_ARR, base->size * len, base->align);
  ty->base = base;
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

  for (Node *n = node->body; n; n = n->next)
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
  case ND_VAR:
    node->ty = node->var->ty;
    return;
  case ND_COMMA:
    node->ty = node->rhs->ty;
    return;
  case ND_MEMBER:
    node->ty = node->member->ty;
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
    return;
  case ND_STMT_EXPR:
  {
    Node *stmt = node->body;
    while (stmt->next)
      stmt = stmt->next;
    node->ty = stmt->lhs->ty;
    return;
  }
  }
}