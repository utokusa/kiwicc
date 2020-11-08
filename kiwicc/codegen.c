#include "kiwicc.h"

/*********************************************
* ...code generator...
*********************************************/

static int top;
static int labelseq = 1;
static int brkseq;
static int contseq;
static Function *current_fn;
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
    printf("  movsxb (%%%s), %%%s\n", rs, rd);
  else if (sz == 2)
    printf("  movsxw (%%%s), %%%s\n", rs, rd);
  else if (sz == 4)
    printf("  movl (%%%s), %%%s\n", rs, rd);
  else
    printf("  mov (%%%s), %%%s\n", rs, rd);
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
      printf("  mov %d(%%%s), %%al\n", i, rs);
      printf("  mov %%al, %d(%%%s)\n", i, rd);
    }
  }
  else if (sz == 1)
    printf("  mov %%%sb, (%%%s)\n", rs, rd);
  else if (sz == 2)
    printf("  mov %%%sw, (%%%s)\n", rs, rd);
  else if (sz == 4)
    printf("  mov %%%sd, (%%%s)\n", rs, rd);
  else
    printf("  mov %%%s, (%%%s)\n", rs, rd);
  top--;
}

static void cast(Type *from, Type *to)
{
  if (to->kind == TY_VOID)
    return;

  char *r = reg(top - 1);

  if (to->kind == TY_BOOL)
  {
    printf("  cmp $0, %%%s\n", r);
    printf("  setne %%%sb\n", r);
    printf("  movzx %%%sb, %%%s\n", r, r);
    return;
  }

  if (size_of(to) == 1)
    printf("  movsx %%%sb, %%%s\n", r, r);
  if (size_of(to) == 2)
    printf("  movsx %%%sw, %%%s\n", r, r);
  if (size_of(to) == 4)
    printf("  mov %%%sd, %%%sd\n", r, r);
  if (is_integer(from) && size_of(from) < 8)
    printf("  movsx %%%sd, %%%s\n", r, r);
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
      printf("  lea -%d(%%rbp), %%%s\n", node->var->offset, reg(top++));
    else
      printf("  mov $%s, %%%s\n", var->name, reg(top++));
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
    printf("  add $%d, %%%s\n", node->member->offset, reg(top - 1));
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

static void divmod(Node *node, char *rd, char *rs, char *r64, char *r32)
{
  if (size_of(node->ty) == 8)
  {
    printf("  mov %%%s, %%rax\n", rd);
    printf("  cqo\n");
    printf("  idiv %%%s\n", rs);
    printf("  mov %%%s, %%%s\n", r64, rd);
  }
  else
  {
    printf("  mov %%%s, %%eax\n", rd);
    printf("  cdq\n");
    printf("  idiv %%%s\n", rs);
    printf("  mov %%%s, %%%s\n", r32, rd);
  }
}

// Initialize va_list.
// Currently we only support up to 6 arguments
// so only initialize gp_offset and reg_save_area
static void builtin_va_start(Node *node)
{
  // For gp_offset.
  int n = 0;
  for (VarList *vl = current_fn->params; vl; vl = vl->next)
    n++;

  // Store the pointer to va_list in rax.
  // Now node->args[0] should be va_list.
  // Note that the pointer to va_list and the pointer to gp_offset are equal.
  printf("  mov -%d(%%rbp), %%rax\n", node->args[0]->offset);
  // Initialize gp_offset.
  // gp_offset holds the byte offset from reg_save_area to where
  // the next available general purpose argument register is saved.
  printf("  movq $%d, (%%rax)\n", n * 8);
  // Initialize reg_save_area.
  // reg_save_area points to the start of the register save area.
  printf("  mov %%rbp, 16(%%rax)\n");
  printf("  subq $80 ,16(%%rax)\n");
  top++;
}

static void gen_expr(Node *node)
{
  printf("  .loc %d %d\n", node->tok->file_no, node->tok->line_no);
  switch (node->kind)
  {
  case ND_NUM:
    printf("  mov $%ld, %%%s\n", node->val, reg(top++));
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
  case ND_COND:
  {
    int seq = labelseq++;
    gen_expr(node->cond);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  je  .L.else.%d\n", seq);
    gen_expr(node->then);
    top--;
    printf("  jmp .L.end.%d\n", seq);
    printf(".L.else.%d:\n", seq);
    gen_expr(node->els);
    printf(".L.end.%d:\n", seq);
    return;
  }
  case ND_NOT:
    gen_expr(node->lhs);
    printf("  cmp $0, %%%s\n", reg(top - 1));
    printf("  sete %%%sb\n", reg(top - 1));
    printf("  movzx %%%sb, %%%s\n", reg(top - 1), reg(top - 1));
    return;
  case ND_BITNOT:
    gen_expr(node->lhs);
    printf("  not %%%s\n", reg(top - 1));
    return;
  case ND_LOGAND:
  {
    int seq = labelseq++;
    gen_expr(node->lhs);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  je  .L.false.%d\n", seq);
    gen_expr(node->rhs);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  je  .L.false.%d\n", seq);
    printf("  mov $1, %%%s\n", reg(top));
    printf("  jmp .L.end.%d\n", seq);
    printf(".L.false.%d:\n", seq);
    printf("  mov $0, %%%s\n", reg(top++));
    printf("  .L.end.%d:\n", seq);
    return;
  }
  case ND_LOGOR:
  {
    int seq = labelseq++;
    gen_expr(node->lhs);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  jne .L.true.%d\n", seq);
    gen_expr(node->rhs);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  jne .L.true.%d\n", seq);
    printf("  mov $0, %%%s\n", reg(top));
    printf("  jmp .L.end.%d\n", seq);
    printf(".L.true.%d:\n", seq);
    printf("  mov $1, %%%s\n", reg(top++));
    printf("  .L.end.%d:\n", seq);
    return;
  }
  case ND_FUNCALL:
  {
    if (!strcmp(node->funcname, "__builtin_va_start"))
    {
      builtin_va_start(node);
      return;
    }

    // So far we only support up to 6 arguments.
    //
    // We should push r10 and r11 becouse they are caller saved registers.
    // RAX is set to 0 for varidic function.
    printf("  push %%r10\n");
    printf("  push %%r11\n");

    // Load arguments from the stack.
    for (int i = 0; i < node->nargs; i++)
    {
      Var *arg = node->args[i];
      int sz = size_of(arg->ty);
      if (sz == 1)
        printf("  movsxb -%d(%%rbp), %%%s\n", arg->offset, argreg32[i]);
      else if (sz == 2)
        printf("  movsxw -%d(%%rbp), %%%s\n", arg->offset, argreg32[i]);
      else if (sz == 4)
        printf("  movl -%d(%%rbp), %%%s\n", arg->offset, argreg32[i]);
      else
        printf("  movq -%d(%%rbp), %%%s\n", arg->offset,  argreg64[i]);
    }

    printf("  mov $0, %%rax\n");
    printf("  call %s\n", node->funcname);

    // The System V x86-64 ABI has a special rule regarding a boolean return
    // value that onlyu the lower 8 bits are valid for it and the upper
    // 56 bits may contain garbage. Here, we clear the upper 56 bits.
    if (node->ty->kind == TY_BOOL)
      printf("  movzx %%al, %%rax\n");

    printf("  pop %%r11\n");
    printf("  pop %%r10\n");
    printf("  mov %%rax, %%%s\n", reg(top++));
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
    printf("  add %%%s, %%%s\n", rs, rd);
    break;
  case ND_PTR_ADD:
    printf("  imul $%d, %%%s\n", node->ty->base->size, rs);
    printf("  add %%%s, %%%s\n", rs, rd);
    break;
  case ND_SUB:
    printf("  sub %%%s, %%%s\n", rs, rd);
    break;
  case ND_PTR_SUB:
    printf("  imul $%d, %%%s\n", node->ty->base->size, rs);
    printf("  sub %%%s, %%%s\n", rs, rd);
    break;
  case ND_PTR_DIFF:
    printf("  sub %%%s, %%%s\n", rs, rd);
    printf("  mov %%%s, %%rax\n", rd);
    printf("  cqo\n");
    printf("  mov $%d, %%%s\n", node->lhs->ty->base->size, rs);
    printf("  idiv %%%s\n", rs);
    printf("  mov %%rax, %%%s\n", rd);
    break;
  case ND_MUL:
    printf("  imul %%%s, %%%s\n", rs, rd);
    break;
  case ND_DIV:
    divmod(node, rd, rs, "rax", "eax");
    break;
  case ND_MOD:
    divmod(node, rd, rs, "rdx", "edx");
    return;
  case ND_BITAND:
    printf("  and %%%s, %%%s\n", rs, rd);
    return;
  case ND_BITOR:
    printf("  or %%%s, %%%s\n", rs, rd);
    return;
  case ND_BITXOR:
    printf("  xor %%%s, %%%s\n", rs, rd);
    return;
  case ND_EQ:
    printf("  cmp %%%s, %%%s\n", rs, rd);
    printf("  sete %%al\n");
    printf("  movzb %%al, %%%s\n", rd);
    break;
  case ND_NE:
    printf("  cmp %%%s, %%%s\n", rs, rd);
    printf("  setne %%al\n");
    printf("  movzb %%al, %%%s\n", rd);
    break;
  case ND_LT:
    printf("  cmp %%%s, %%%s\n", rs, rd);
    printf("  setl %%al\n");
    printf("  movzb %%al, %%%s\n", rd);
    break;
  case ND_LE:
    printf("  cmp %%%s, %%%s\n", rs, rd);
    printf("  setle %%al\n");
    printf("  movzb %%al, %%%s\n", rd);
    break;
  case ND_SHL:
    printf("  mov %%%s, %%rcx\n", reg(top));
    printf("  shl %%cl, %%%s\n", rd);
    return;
  case ND_SHR:
    printf("  mov %%%s, %%rcx\n", reg(top));
    printf("  sar %%cl, %%%s\n", rd);
    return;
  default:
    error_tok(node->tok, "invalid statement");
  }
}

static void gen_stmt(Node *node)
{
  printf("  .loc %d %d\n", node->tok->file_no, node->tok->line_no);

  switch (node->kind)
  {
  case ND_IF:
  {
    int seq = labelseq++;
    if (node->els)
    {
      gen_expr(node->cond);
      printf("  cmp $0, %%%s\n", reg(--top));
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
      printf("  cmp $0, %%%s\n", reg(--top));
      printf("  je .L.end.%d\n", seq);
      gen_stmt(node->then);
      printf(".L.end.%d:\n", seq);
    }
    return;
  }
  case ND_WHILE:
  {
    int seq = labelseq++;
    int brk = brkseq;
    int cont = contseq;
    brkseq = contseq = seq;

    printf(".L.begin.%d:\n", seq);
    gen_expr(node->cond);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  je .L.break.%d\n", seq);
    gen_stmt(node->then);
    printf(".L.continue.%d:\n", seq);
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.break.%d:\n", seq);

    brkseq = brk;
    contseq = cont;
    return;
  }
  case ND_DO:
  {
    int seq = labelseq++;
    int brk = brkseq;
    int cont = contseq;
    brkseq = contseq = seq;

    printf(".L.begin.%d:\n", seq);
    gen_stmt(node->then);
    printf(".L.continue.%d:\n", seq);
    gen_expr(node->cond);
    printf("  cmp $0, %%%s\n", reg(--top));
    printf("  jne .L.begin.%d\n", seq);
    printf(".L.break.%d:\n", seq);

    brkseq = brk;
    contseq = cont;
    return;
  }
  case ND_FOR:
  {
    int seq = labelseq++;
    int brk = brkseq;
    int cont = contseq;
    brkseq = contseq = seq;

    if (node->init)
      gen_stmt(node->init);
    printf(".L.begin.%d:\n", seq);
    if (node->cond)
    {
      gen_expr(node->cond);
      printf("  cmp $0, %%%s\n", reg(--top));
      printf("  je .L.break.%d\n", seq);
    }
    gen_stmt(node->then);
    printf(".L.continue.%d:\n", seq);
    if (node->inc)
      gen_stmt(node->inc);
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.break.%d:\n", seq);

    brkseq = brk;
    contseq = cont;
    return;
  }
  case ND_SWITCH:
  {
    int seq = labelseq++;
    int brk = brkseq;
    brkseq = seq;
    node->case_label = seq;

    gen_expr(node->cond);

    for (Node *n = node->case_next; n; n = n->case_next)
    {
      n->case_label = labelseq++;
      printf("  cmp $%ld, %%%s\n", n->val, reg(top - 1));
      printf("  je .L.case.%d\n", n->case_label);
    }
    top--;

    if (node->default_case)
    {
      int i = labelseq++;
      node->default_case->case_label = i;
      printf("  jmp .L.case.%d\n", i);
    }

    printf("  jmp .L.break.%d\n", seq);
    gen_stmt(node->then);
    printf(".L.break.%d:\n", seq);

    brkseq = brk;
    return;
  }
  case ND_CASE:
    printf(".L.case.%d:\n", node->case_label);
    gen_stmt(node->lhs);
    return;
  case ND_BLOCK:
  {
    for (Node *cur = node->body; cur; cur = cur->next)
      gen_stmt(cur);
    return;
  }
  case ND_BREAK:
    if (brkseq == 0)
      error_tok(node->tok, "stray break");
    printf("  jmp .L.break.%d\n", brkseq);
    return;
  case ND_CONTINUE:
    if (contseq == 0)
      error_tok(node->tok, "stray continue");
    printf("  jmp .L.continue.%d\n", contseq);
    return;
  case ND_GOTO:
    printf("  jmp .L.label.%s.%s\n", current_fn->name, node->label_name);
    return;
  case ND_LABEL:
    printf(".L.label.%s.%s:\n", current_fn->name, node->label_name);
    gen_stmt(node->lhs);
    return;
  case ND_RETURN:
    if (node->lhs)
    {
      gen_expr(node->lhs);
      printf("  mov %%%s, %%rax\n", reg(--top));
    }
    printf("  jmp .L.return.%s\n", current_fn->name);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    top--;
    return;
  default:
    error_tok(node->tok, "invalid expression");
  }
}

static void emit_bss(Program *prog)
{
  printf(".bss\n");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;

    if (var->init_data)
      continue;

    printf("  .align %d\n", var->align);
    if (!var->is_static)
      printf("  .globl %s\n", var->name);
    printf("%s:\n", var->name);
    printf("  .zero %d\n", size_of(var->ty));
  }
}

static void emit_data(Program *prog)
{
  printf(".data\n");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;
    if (!var->init_data)
      continue;

    printf("  .align %d\n", var->align);
    if (!var->is_static)
      printf("  .globl %s\n", var->name);
    printf("%s:\n", var->name);
    Relocation *rel = var->rel;
    int pos = 0;
    while (pos < var->ty->size)
    {
      if (rel && rel->offset == pos)
      {
        printf("  .quad %s%+ld\n", rel->label, rel->addend);
        rel = rel->next;
        pos += 8;
      }
      else
        printf("  .byte %d\n", var->init_data[pos++]);
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
    current_fn = fn;
    if (!fn->is_static)
      printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);

    // Prologue. r12-15 are callee-saved registers.
    printf("  push %%rbp\n");
    printf("  mov %%rsp, %%rbp \n");
    printf("  sub $%d, %%rsp\n", fn->stack_size);
    printf("  mov %%r12, -8(%%rbp)\n");
    printf("  mov %%r13, -16(%%rbp)\n");
    printf("  mov %%r14, -24(%%rbp)\n");
    printf("  mov %%r15, -32(%%rbp)\n");

    // Save arg registers to the register save area
    // if the function is the variadic
    if (fn->is_variadic)
    {
      printf("  mov %%rdi, -80(%%rbp)\n");
      printf("  mov %%rsi, -72(%%rbp)\n");
      printf("  mov %%rdx, -64(%%rbp)\n");
      printf("  mov %%rcx, -56(%%rbp)\n");
      printf("  mov %%r8, -48(%%rbp)\n");
      printf("  mov %%r9, -40(%%rbp)\n");
    }

    // Save arguments to the stack
    int i = 0;
    for (VarList *param = fn->params; param; param = param->next)
      i++;

    for (VarList *param = fn->params; param; param = param->next)
    {
      Var *var = param->var;
      char *r = get_argreg(size_of(var->ty), --i);
      printf("  mov %%%s, -%d(%%rbp)\n", r, param->var->offset);
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
    printf("  mov -8(%%rbp), %%r12\n");
    printf("  mov -16(%%rbp), %%r13\n");
    printf("  mov -24(%%rbp), %%r14\n");
    printf("  mov -32(%%rbp), %%r15\n");
    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");
    printf("  ret\n");
  }
}

void codegen(Program *prog)
{
  // Output the assembly code.

  char **paths = get_input_files();
  for (int i = 0; paths[i]; i++)
    printf("  .file %d \"%s\"\n", i + 1, paths[i]);

  emit_bss(prog);
  emit_data(prog);
  emit_text(prog);
}
