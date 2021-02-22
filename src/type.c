#include "kiwicc.h"

/*********************************************
* ...type...
*********************************************/

Type *void_type = &(Type){TY_VOID, 1, 1};
Type *bool_type = &(Type){TY_BOOL, 1, 1};
Type *char_type = &(Type){TY_CHAR, 1, 1};
Type *short_type = &(Type){TY_SHORT, 2, 2};
Type *int_type = &(Type){TY_INT, 4, 4};
Type *long_type = &(Type){TY_LONG, 8, 8};
Type *float_type = &(Type){TY_FLOAT, 4, 4};
Type *double_type = &(Type){TY_DOUBLE, 8, 8};
Type *uchar_type = &(Type){TY_CHAR, 1, 1, true};
Type *ushort_type = &(Type){TY_SHORT, 2, 2, true};
Type *uint_type = &(Type){TY_INT, 4, 4, true};
Type *ulong_type = &(Type){TY_LONG, 8, 8, true};

static Type *new_type(TypeKind kind, int size, int align)
{
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = kind;
  ty->size = size;
  ty->align = align;
  return ty;
}

bool is_integer(Type *ty)
{
  TypeKind k = ty->kind;
  return k == TY_BOOL || k == TY_CHAR || k == TY_SHORT ||
         k == TY_INT || k == TY_LONG || k == TY_ENUM;
}

bool is_flonum(Type *ty)
{
  TypeKind k = ty->kind;
  return k == TY_FLOAT || k == TY_DOUBLE;
}

bool is_numeric(Type *ty)
{
  return is_integer(ty) || is_flonum(ty);
}

static bool is_scalar(Type *ty)
{
  return is_numeric(ty) || ty->base;
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

int align_down(int n, int align)
{
  return align_to(n - align + 1, align);
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

int size_of(Type *ty)
{
  if (ty->kind == TY_VOID)
    error_tok(ty->name, "void type");
  if (ty->is_incomplete)
    error_tok(ty->name, "incomplete type"); // e.g. "int a[]; sizeof(a);"
  return ty->size;
}

static Type *get_common_type(Type *ty1, Type *ty2)
{
  if (ty1->base)
    return pointer_to(ty1->base);
  
  if (ty1->kind == TY_DOUBLE || ty2->kind == TY_DOUBLE)
    return double_type;
  if (ty1->kind == TY_FLOAT || ty2->kind == TY_FLOAT)
    return float_type;
  
  if (ty1->size < 4)
    ty1 = int_type;
  if (ty2->size < 4)
    ty2 = int_type;
  
  if (ty1->size != ty2->size)
    return (ty1->size < ty2->size) ? ty2 : ty1;
  
  if (ty2->is_unsigned)
    return ty2;
  return ty1;
}

// Usual arithmettic conversion
static void usual_arith_conv(Node **lhs, Node **rhs)
{
  Type *ty = get_common_type((*lhs)->ty, (*rhs)->ty);
  *lhs = new_cast(*lhs, ty);
  *rhs = new_cast(*rhs, ty);
}

Type *array_of(Type *base, int len)
{
  Type *ty = new_type(TY_ARR, size_of(base) * len, base->align);
  ty->base = base;
  ty->array_len = len;
  return ty;
}

Type *enum_type()
{
  return new_type(TY_ENUM, 4, 4);
}

Type *struct_type()
{
  return new_type(TY_STRUCT, 0, 1);
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
  case ND_NUM:
    node->ty = int_type;
    return;
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_MOD:
  case ND_BITAND:
  case ND_BITOR:
  case ND_BITXOR:
    usual_arith_conv(&node->lhs, &node->rhs);
    node->ty = node->lhs->ty;
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    usual_arith_conv(&node->lhs, &node->rhs);
    node->ty = int_type;
    return;
  case ND_PTR_DIFF:
    node->ty = long_type;
    return;
  case ND_PTR_ADD:
  case ND_PTR_SUB:
    node->ty = node->lhs->ty;
    return;
  case ND_ASSIGN:
    if (is_scalar(node->rhs->ty))
      node->rhs = new_cast(node->rhs, node->lhs->ty);
    node->ty = node->lhs->ty;
    return;
  case ND_NOT:
  case ND_LOGAND:
  case ND_LOGOR:
    node->ty = int_type;
    return;
  case ND_BITNOT:
  case ND_SHL:
  case ND_SHR:
    node->ty = node->lhs->ty;
    return;
  case ND_VAR:
    node->ty = node->var->ty;
    return;
  case ND_COND:
    if (node->then->ty->kind == TY_VOID || node->els->ty->kind == TY_VOID)
      node->ty = void_type;
    else
    {
      usual_arith_conv(&node->then, &node->els);
      node->ty = node->then->ty;
    }
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
    if (!node->lhs->ty->base)
      error_tok(node->tok, "invalid pointer dereference");
    if (node->lhs->ty->base->kind == TY_VOID)
      error_tok(node->tok, "dereferencing a void pointer");
    node->ty = node->lhs->ty->base;
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
