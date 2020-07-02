#include "9cc.h"

/*********************************************
* ...code generator...
*********************************************/

static int top;
static int labelseq = 1;
static char *funcname;
static char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static char *reg(int idx)
{
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: &d\n", idx);
  return r[idx];
}

static char *xreg(Type *ty, int idx)
{
  if (ty->base || size_of(ty) == 8)
    return reg(idx);

  static char *r[] = {"r10d", "r11d", "r12d", "r13d", "r14d", "r15d"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

static void load(Type *ty)
{
  if (ty->kind == TY_ARR || ty->kind == TY_STRUCT)
    return;

  char *rs = reg(top - 1);
  char *rd = xreg(ty, top - 1);
  int sz = size_of(ty);

  if (sz == 1)
    printf("  movsx %s, byte ptr [%s]\n", rd, rs);
  else if (sz == 2)
    printf("  movsx %s, word ptr [%s]\n", rd, rs);
  else if (sz == 4)
    printf("  mov %s, dword ptr [%s]\n", rd, rs);
  else
    printf("  mov %s, [%s]\n", rd, rs);
}

static void store(Type *ty)
{
  char *rd = reg(top - 1);
  char *rs = reg(top - 2);
  int sz = size_of(ty);

  if (ty->kind == TY_STRUCT)
  {
    for (int i = 0; i < sz; i++)
    {
      printf("  mov al, [%s+%d]\n", rs, i);
      printf("  mov [%s+%d], al\n", rd, i);
    }
  }
  else if (sz == 1)
    printf("  mov [%s], %sb\n", rd, rs);
  else if (sz == 2)
    printf("  mov [%s], %sw\n", rd, rs);
  else if (sz == 4)
    printf("  mov [%s], %sd\n", rd, rs);
  else
    printf("  mov [%s], %s\n", rd, rs);
  top--;
}

static void cast(Type *from, Type *to)
{
  if (to->kind == TY_VOID)
    return;

  char *r = reg(top - 1);

  if (to->kind == TY_BOOL)
  {
    printf("  cmp %s, 0\n", r);
    printf("  setne %sb\n", r);
    printf("  movzx %s, %sb\n", r, r);
    return;
  }

  if (size_of(to) == 1)
    printf("  movsx %s, %sb\n", r, r);
  if (size_of(to) == 2)
    printf("  movsx %s, %sw\n", r, r);
  if (size_of(to) == 4)
    printf("  mov %sd, %sd\n", r, r);
  if (is_integer(from) && size_of(from) < 8)
    printf("  movsx %s, %sd\n", r, r);
}

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

// push the given node's addresss to the stack
static void gen_addr(Node *node)
{
  switch (node->kind)
  {
  case ND_VAR:
  {
    Var *var = node->var;
    if (var->is_local)
      printf("  lea %s, [rbp-%d]\n", reg(top++), node->var->offset);
    else
      printf("  mov %s, offset %s\n", reg(top++), var->name);
    return;
  }
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  case ND_COMMA:
    gen_expr(node->lhs);
    top--;
    gen_addr(node->rhs);
    return;
  case ND_MEMBER:
    gen_addr(node->lhs);
    printf("  add %s, %d\n", reg(top - 1), node->member->offset);
    return;
  default:
    printf("node->kind : %d\n", node->kind);
    error_tok(node->tok, "The lvalue of the assignment is not a variable.");
  }
}

static void gen_lval(Node *node)
{
  if (node->ty->kind == TY_ARR)
    error_tok(node->tok, "not an lvalue");
  gen_addr(node);
}

static void gen_expr(Node *node)
{
  printf(".loc 1 %d\n", node->tok->line_no);
  switch (node->kind)
  {
  case ND_NUM:
    printf("  mov %s, %d\n", reg(top++), node->val);
    return;
  case ND_VAR:
  case ND_MEMBER:
    gen_addr(node);
    load(node->ty);
    return;
  case ND_ASSIGN:
    gen_expr(node->rhs);
    gen_lval(node->lhs);
    store(node->ty);
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    load(node->ty);
    return;
  case ND_STMT_EXPR:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    top++;
    return;
  case ND_NULL_EXPR:
    top++;
    return;
  case ND_COMMA:
    gen_expr(node->lhs);
    top--;
    gen_expr(node->rhs);
    return;
  case ND_CAST:
    gen_expr(node->lhs);
    cast(node->lhs->ty, node->ty);
    return;
  case ND_FUNCALL:
  {
    // So far we only support up to 6 arguments.
    //
    // We should push r10 and r11 becouse they are caller saved registers.
    // RAX is set to 0 for varidic function.
    printf("  push r10\n");
    printf("  push r11\n");

    // Load arguments from the stack.
    for (int i = 0; i < node->nargs; i++)
    {
      Var *arg = node->args[i];
      int sz = size_of(arg->ty);
      if (sz == 1)
        printf("  movsx %s, byte ptr [rbp-%d]\n", argreg32[i], arg->offset);
      else if (sz == 2)
        printf("  movsx %s, word ptr [rbp-%d]\n", argreg32[i], arg->offset);
      else if (sz == 4)
        printf("  mov %s, dword ptr [rbp-%d]\n", argreg32[i], arg->offset);
      else
        printf("  mov %s, [rbp-%d]\n", argreg64[i], arg->offset);
    }

    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  pop r11\n");
    printf("  pop r10\n");
    printf("  mov %s, rax\n", reg(top++));
    return;
  }
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

  char *rd = xreg(node->lhs->ty, top - 2);
  char *rs = xreg(node->lhs->ty, top - 1);
  top--;

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add %s, %s\n", rd, rs);
    break;
  case ND_PTR_ADD:
    printf("  imul %s, %d\n", rs, node->ty->base->size);
    printf("  add %s, %s\n", rd, rs);
    break;
  case ND_SUB:
    printf("  sub %s, %s\n", rd, rs);
    break;
  case ND_PTR_SUB:
    printf("  imul %s, %d\n", rs, node->ty->base->size);
    printf("  sub %s, %s\n", rd, rs);
    break;
  case ND_PTR_DIFF:
    printf("  sub %s, %s\n", rd, rs);
    printf("  mov rax, %s\n", rd);
    printf("  cqo\n");
    printf("  mov %s, %d\n", rs, node->lhs->ty->base->size);
    printf("  idiv %s\n", rs);
    printf("  mov %s, rax\n", rd);
    break;
  case ND_MUL:
    printf("  imul %s, %s\n", rd, rs);
    break;
  case ND_DIV:
    if (size_of(node->ty) == 8)
    {
      printf("  mov rax, %s\n", rd);
      printf("  cqo\n");
      printf("  idiv %s\n", rs);
      printf("  mov %s, rax\n", rd);
    }
    else
    {
      printf("  mov eax, %s\n", rd);
      printf("  cdq\n");
      printf("  idiv %s\n", rs);
      printf("  mov %s, eax\n", rd);
    }
    break;
  case ND_EQ:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  sete al\n");
    printf("  movzb %s, al\n", rd);
    break;
  case ND_NE:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setne al\n");
    printf("  movzb %s, al\n", rd);
    break;
  case ND_LT:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setl al\n");
    printf("  movzb %s, al\n", rd);
    break;
  case ND_LE:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setle al\n");
    printf("  movzb %s, al\n", rd);
    break;
  default:
    error_tok(node->tok, "invalid statement");
  }
}

static void gen_stmt(Node *node)
{
  printf(".loc 1 %d\n", node->tok->line_no);

  switch (node->kind)
  {
  case ND_IF:
  {
    int seq = labelseq++;
    if (node->els)
    {
      gen_expr(node->cond);
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.else.%d\n", seq);
      gen_stmt(node->then);
      printf("  jmp .L.end.%d\n", seq);
      printf(".L.else.%d:\n", seq);
      gen_stmt(node->els);
      printf(".L.end.%d:\n", seq);
    }
    else
    {
      gen_expr(node->cond);
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.end.%d\n", seq);
      gen_stmt(node->then);
      printf(".L.end.%d:\n", seq);
    }
    return;
  }
  case ND_WHILE:
  {
    int seq = labelseq++;

    printf(".L.begin.%d:\n", seq);
    gen_expr(node->cond);
    printf("  cmp %s, 0\n", reg(--top));
    printf("  je .L.end.%d\n", seq);
    gen_stmt(node->then);
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.end.%d:\n", seq);
    return;
  }
  case ND_FOR:
  {
    int seq = labelseq++;

    if (node->init)
      gen_stmt(node->init);
    printf(".L.begin.%d:\n", seq);
    if (node->cond)
    {
      gen_expr(node->cond);
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.end.%d\n", seq);
    }
    gen_stmt(node->then);
    if (node->inc)
      gen_stmt(node->inc);
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.end.%d:\n", seq);
    return;
  }
  case ND_BLOCK:
  {
    for (Node *cur = node->body; cur; cur = cur->next)
      gen_stmt(cur);
    return;
  }
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("  mov rax, %s\n", reg(--top));
    printf("  jmp .L.return.%s\n", funcname);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    top--;
    return;
  default:
    error_tok(node->tok, "invalid expression");
  }
}

static void emit_data(Program *prog)
{
  printf(".data\n");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;
    printf("%s:\n", var->name);

    if (!var->init_data)
    {
      printf("  .zero %d\n", size_of(var->ty));
      continue;
    }

    for (int i = 0; i < size_of(var->ty); ++i)
    {
      printf("  .byte %d\n", var->init_data[i]);
    }
  }
}

static char *get_argreg(int sz, int idx)
{
  if (sz == 1)
    return argreg8[idx];
  if (sz == 2)
    return argreg16[idx];
  if (sz == 4)
    return argreg32[idx];
  assert(sz == 8);
  return argreg64[idx];
}

static void emit_text(Program *prog)
{
  printf(".text\n");
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    funcname = fn->name;
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);

    // Prologue. r12-15 are callee-saved registers.
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);
    printf("  mov [rbp-8], r12\n");
    printf("  mov [rbp-16], r13\n");
    printf("  mov [rbp-24], r14\n");
    printf("  mov [rbp-32], r15\n");

    // Save arguments to the stack
    int i = 0;
    for (VarList *param = fn->params; param; param = param->next)
      i++;

    for (VarList *param = fn->params; param; param = param->next)
    {
      Var *var = param->var;
      char *r = get_argreg(size_of(var->ty), --i);
      printf("  mov [rbp-%d], %s\n", param->var->offset, r);
    }

    // Generate statements
    for (Node *node = fn->node; node; node = node->next)
    {
      gen_stmt(node);
      assert(top == 0);
    }

    // Epilogue
    // Restore the values of r12 to r15.
    // Note that we don't have to restore r10, r11
    // because they are caller-saved registers.
    printf(".L.return.%s:\n", fn->name);
    printf("  mov r12, [rbp-8]\n");
    printf("  mov r13, [rbp-16]\n");
    printf("  mov r14, [rbp-24]\n");
    printf("  mov r15, [rbp-32]\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}

void codegen(Program *prog)
{
  // Output the assembly code.
  printf(".intel_syntax noprefix\n");
  emit_data(prog);
  emit_text(prog);
}