#include "9cc.h"

/*********************************************
* ...code generator...
*********************************************/

static int top;
static int labelseq = 1;
static char *funcname;
static char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static char *reg(int idx)
{
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: &d\n", idx);
  return r[idx];
}

static void load(Type *ty)
{
  char *r = reg(top - 1);
  if (ty->size == 1)
    printf("  movsx %s, byte ptr [%s]\n", r, r);
  else
    printf("  mov %s, [%s]\n", r, r);
}

static void store(Type *ty)
{
  char *rd = reg(top - 1);
  char *rs = reg(top - 2);

  if (ty->size == 1)
    printf("  mov [%s], %sb\n", rd, rs);
  else
    printf("  mov [%s], %s\n", rd, rs);
  top--;
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
    if (node->ty->kind != TY_ARR)
      load(node->ty);
    return;
  case ND_SIZEOF:
    printf("  mov %s, %d\n", reg(top++), node->lhs->ty->size);
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
    if (node->ty->kind != TY_ARR)
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
      if (arg->ty->size == 1)
        printf("  movsx %s, byte ptr [rbp-%d]\n", argreg64[i], arg->offset);
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

  char *rd = reg(top - 2);
  char *rs = reg(top - 1);
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
    printf("  mov rax, %s\n", rd);
    printf("  cqo\n");
    printf("  idiv %s\n", rs);
    printf("  mov %s, rax\n", rd);
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
      printf("  .zero %d\n", var->ty->size);
      continue;
    }

    for (int i = 0; i < var->ty->size; ++i)
    {
      printf("  .byte %d\n", var->init_data[i]);
    }
  }
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
    int nargs = 0;
    for (VarList *param = fn->params; param && nargs < 6; param = param->next)
    {
      Var *var = param->var;
      if (var->ty->size == 1)
        printf("  mov [rbp-%d], %s\n", param->var->offset, argreg8[nargs++]);
      else
        printf("  mov [rbp-%d], %s\n", param->var->offset, argreg64[nargs++]);
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