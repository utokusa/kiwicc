#include "9cc.h"

/*********************************************
* ...code generator...
*********************************************/

static int top;
static int labelseq = 1;
static char *funcname;
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static char *reg(int idx)
{
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: &d\n", idx);
  return r[idx];
}

static void load()
{
  printf("  mov %s, [%s]\n", reg(top - 1), reg(top - 1));
}

static void store()
{
  printf("  mov [%s], %s\n", reg(top - 1), reg(top - 2));
  top--;
}

static void gen(Node *node);

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
    gen(node->lhs);
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

static void gen(Node *node)
{
  switch (node->kind)
  {
  case ND_NUM:
    printf("  mov %s, %d\n", reg(top++), node->val);
    return;
  case ND_VAR:
    gen_addr(node);
    if (node->ty->kind != TY_ARR)
      load();
    return;
  case ND_NULL:
    return;
  case ND_SIZEOF:
    printf("  mov %s, %d\n", reg(top++), node->lhs->ty->size);
    return;
  case ND_ASSIGN:
    gen(node->rhs);
    gen_lval(node->lhs);
    store();
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    if (node->ty->kind != TY_ARR)
      load();
    return;
  case ND_IF:
  {
    int seq = labelseq++;
    if (node->els)
    {
      gen(node->cond);
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.else.%d\n", seq);
      gen(node->then);
      printf("  jmp .L.end.%d\n", seq);
      printf(".L.else.%d:\n", seq);
      gen(node->els);
      printf(".L.end.%d:\n", seq);
    }
    else
    {
      gen(node->cond);
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.end.%d\n", seq);
      gen(node->then);
      printf(".L.end.%d:\n", seq);
    }
    return;
  }
  case ND_WHILE:
  {
    int seq = labelseq++;

    printf(".L.begin.%d:\n", seq);
    gen(node->cond);
    printf("  cmp %s, 0\n", reg(--top));
    printf("  je .L.end.%d\n", seq);
    gen(node->then);
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.end.%d:\n", seq);
    return;
  }
  case ND_FOR:
  {
    int seq = labelseq++;

    if (node->init)
      gen(node->init);
    printf(".L.begin.%d:\n", seq);
    if (node->cond)
    {
      gen(node->cond);
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.end.%d\n", seq);
    }
    gen(node->then);
    if (node->inc)
      gen(node->inc);
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.end.%d:\n", seq);
    return;
  }
  case ND_BLOCK:
  {
    for (Node *cur = node->block; cur; cur = cur->next)
      gen(cur);
    return;
  }
  case ND_RETURN:
    gen(node->lhs);
    printf("  mov rax, %s\n", reg(--top));
    printf("  jmp .L.return.%s\n", funcname);
    return;
  case ND_FUNCALL:
  {
    // So far we only support up to 6 arguments.
    int nargs = 0;
    for (Node *arg = node->arg; arg && nargs < 6; arg = arg->next)
    {
      gen(arg);
      nargs++;
    }
    for (int i = nargs - 1; i >= 0; --i)
      printf("  mov %s, %s\n", argreg[i], reg(--top));

    // We should push r10 and r11 becouse they are caller saved registers.
    // RAX is set to 0 for varidic function.
    int seq = labelseq++;
    printf("  push r10\n");
    printf("  push r11\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  pop r11\n");
    printf("  pop r10\n");
    printf("  mov %s, rax\n", reg(top++));
    return;
  }
  case ND_EXPR_STMT:
    gen(node->lhs);
    top--;
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

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

static void emit_data(Program *prog)
{
  printf(".data\n");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;
    printf("%s:\n", var->name);
    printf("  .zero %d\n", var->ty->size);
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
      printf("  mov [rbp-%d], %s\n", param->var->offset, argreg[nargs++]);

    // Generate statements
    for (Node *node = fn->node; node; node = node->next)
    {
      gen(node);
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