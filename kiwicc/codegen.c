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
static char *argreg[] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
static char *fargreg[] = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};

static char *reg(int idx)
{
  static char *r[] = {"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

static char *xreg(Type *ty, int idx)
{
  if (ty->base || size_of(ty) == 8)
    return reg(idx);

  static char *r[] = {"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

static char *freg(int idx)
{
  static char *r[] = {"xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

// In RISC-V, `addi` can take only sign-extended 12-bit immediate [-2048, 2047].
// This function allows to take larger/smaller immediates.
static void gen_addi(char *rd, char *rs, long imm)
{
  if (-2048 <= imm && imm <= 2047)
  {
    println("  addi %s, %s, %ld", rd, rs, imm);
    return;
  }

  println("  li t1, %ld", imm);
  println("  add %s, %s, t1", rd, rs);
}

// For `lb`, `ls`, `lw`, `ld`, `sb`, `ss`, `sw`, `sd`
static void gen_offset_instr(char *instr, char *rd, char *r1, long offset)
{
  // If offset can be represented as sign-extended 12-bit
  // immediate [-2048, 2047].
  if (-2048 <= offset && offset <= 2047)
  {
    println("  %s %s, %ld(%s)", instr, rd, offset, r1);
    return;
  }

  println("  li t1, %ld", offset);
  println("  add t2, %s, t1", r1);
  println("  %s %s, (t2)", instr, rd);
}

static void load(Type *ty)
{
  println("# load");
  if (ty->kind == TY_ARR || ty->kind == TY_STRUCT || ty->kind == TY_FUNC)
    return;
  
  if (ty->kind == TY_FLOAT)
  {
    println("  movss (%%%s), %%%s", reg(top - 1), freg(top - 1));
    return;
  }
  if (ty->kind == TY_DOUBLE)
  {
    println("  movsd (%%%s), %%%s", reg(top - 1), freg(top - 1));
    return;
  }

  char *rs = reg(top - 1);
  char *rd = xreg(ty, top - 1);
  char *movop = ty->is_unsigned ? "movz" : "movs";
  int sz = size_of(ty);

  if (sz == 1)
    println("  lb %s, (%s)", rd, rs);
  else if (sz == 2)
    println("  lh %s, (%s)", rd, rs);
  else if (sz == 4)
    println("  lw %s, (%s)", rd, rs);
  else
    println("  ld %s, (%s)", rd, rs);
}

static void store(Type *ty)
{
  println("# store");
  char *rd = reg(top - 1);
  char *rs = reg(top - 2);
  int sz = size_of(ty);

  if (ty->kind == TY_STRUCT)
  {
    for (int i = 0; i < sz; i++)
    {
      println("  lb a0, %d(%s)", i, rs);
      println("  sb a0, %d(%s)", i, rd);
    }
  }
  else if (ty->kind == TY_FLOAT)
    println("  movss %%%s, (%%%s)", freg(top - 2), rd);
  else if (ty->kind == TY_DOUBLE)
    println("  movsd %%%s, (%%%s)", freg(top - 2), rd);
  else if (sz == 1)
    println("  sb %s, (%s)", rs, rd);
  else if (sz == 2)
    println("  sh %s, (%s)", rs, rd);
  else if (sz == 4)
    println("  sw %s, (%s)", rs, rd);
  else
    println("  sd %s, (%s)", rs, rd);
  top--;
}

static void cmp_zero(Type * ty)
{
  if (ty->kind == TY_FLOAT)
  {
    // Set all of the single-precision floating-point values in xmm0 to zero.
    println("  xorps %%xmm0, %%xmm0");
    println("  ucomiss %%xmm0, %%%s", freg(--top));
  }
  else if (ty->kind == TY_DOUBLE)
  {
    // Set all of the single-precision floating-point values in xmm0 to zero.
    println("  xorpd %%xmm0, %%xmm0");
    println("  ucomisd %%xmm0, %%%s", freg(--top));
  }
  else
  {
    char *rd = reg(--top);
    char *rs = rd;
    println("  seqz %s, %s", rd, rs);
  }

}

static void cast(Type *from, Type *to)
{
  if (to->kind == TY_VOID)
    return;

  char *r = reg(top - 1);
  char *fr = freg(top - 1);

  if (to->kind == TY_BOOL)
  {
    cmp_zero(from);
    println("  setne %%%sb", reg(top));
    println("  movzx %%%sb, %%%s", reg(top), reg(top));
    top++;
    return;
  }

  if (from->kind == TY_FLOAT)
  {
    if (to->kind == TY_FLOAT)
      return;
    
    if (to->kind == TY_DOUBLE)
      println("  cvtss2sd %%%s, %%%s", fr, fr);
    else /* integer */
      println("  cvttss2si %%%s, %%%s", fr, r);
    return;
  }

  if (from->kind == TY_DOUBLE)
  {
    if (to->kind == TY_DOUBLE)
      return;
    
    if (to->kind == TY_FLOAT)
      println("  cvtsd2ss %%%s, %%%s", fr, fr);
    else /* integer */
      println("  cvttsd2si %%%s, %%%s", fr, r);
    return;
  }

  if (to->kind == TY_FLOAT)
  {
    println("  cvtsi2ss %%%s, %%%s", r, fr);
    return;
  }

  if (to->kind == TY_DOUBLE)
  {
    println("  cvtsi2sd %%%s, %%%s", r, fr);
    return;
  }

  // TODO: handle unsigned. The dist register is sign-extended.
  char *suffix = to->is_unsigned ? "u" : "";

  if (size_of(to) == 1)
  {
    println("  addi sp, sp, -8");
    println("  sd %s, (sp)", r);
    println("  lb%s %s, (sp)", suffix, r);
    println("  addi sp, sp, 8");
  }
  else if (size_of(to) == 2)
  {
    println("  addi sp, sp, -8");
    println("  sd %s, (sp)", r);
    println("  lh%s %s, (sp)", suffix, r);
    println("  addi sp, sp, 8");
  }
  else if (size_of(to) == 4)
  {
    println("  addi sp, sp, -8");
    println("  sd %s, (sp)", r);
    println("  lw%s %s, (sp)", suffix, r);
    println("  addi sp, sp, 8");
  }
  else if (is_integer(from) && size_of(from) < 8 && !from->is_unsigned)
    println("  mv %s, %s", r, r);
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
    println("# gen_addr() / ND_VAR");
    Var *var = node->var;
    if (var->is_local)
    {
      gen_addi(reg(top++), "s0", -1 * var->offset);
      return;
    }
    
    if (!opt_fpic)
    {
      println("  la %s, %s", reg(top++), var->name);
    }
    else if (var->is_static)
    {
      // Set %RIP+addend to a register.
      println("  lea %s(%%rip), %%%s", var->name, reg(top++));
    }
    else
    {
      // Load a 64-bit address value from memory and set it to a register.
      println("  mov %s@GOTPCREL(%%rip), %%%s", var->name, reg(top++));
    }
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
  {
    println("# gen_addr() ND_MEMBER start");
    gen_addr(node->lhs);
    char *rd = reg(top - 1);
    gen_addi(rd, rd, node->member->offset);
    println("# gen_addr() ND_MEMBER end");
    return;
  }
  default:
    println("node->kind : %d", node->kind);
    error_tok(node->tok, "The lvalue of the assignment is not a variable.");
  }
}

static void gen_lval(Node *node)
{
  gen_addr(node);
}

static void divmod(Node *node, char *rd, char *rs, char *r64, char *r32)
{
  if (size_of(node->ty) == 8)
  {
    println("  mov %%%s, %%rax", rd);
    if (node->ty->is_unsigned) {
      println("  mov $0, %%rdx");
      println("  div %%%s", rs);
    }
    else {
      println("  cqo");
      println("  idiv %%%s", rs);
    }
    println("  mov %%%s, %%%s", r64, rd);
  }
  else
  {
    println("  mov %%%s, %%eax", rd);
    if (node->ty->is_unsigned) {
      println("  mov $0, %%edx");
      println("  div %%%s", rs);
    }
    else {
      println("  cdq");
      println("  idiv %%%s", rs);
    }
    println("  mov %%%s, %%%s", r32, rd);
  }
}

// Initialize va_list.
/*
https://www.uclibc.org/docs/psABI-x86_64.pdf
typedef struct {
  unsigned int gp_offset;
  unsigned int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} va_list[1];
*/

// Currently we only support up to 6 arguments
// so only initialize gp_offset and reg_save_area
static void builtin_va_start(Node *node)
{
  // g stands for general purpose registers.
  // f stands for floating point registers
  int gp = 0, fp = 0;

  for (VarList *vl = current_fn->params; vl; vl = vl->next)
  {
    Var *var = vl->var;
    if (is_flonum(var->ty))
      fp++;
    else
      gp++;
  }

  // Store the pointer to va_list in rax.
  // Now node->args[0] should be va_list.
  // Note that the pointer to va_list and the pointer to gp_offset are equal.
  println("  mov -%d(%%rbp), %%rax", node->args[0]->offset);
  // Initialize gp_offset.
  // gp_offset holds the byte offset from reg_save_area to where
  // the next available general purpose argument register is saved.
  println("  movl $%d, (%%rax)", gp * 8);
  // Initialize fp_offset.
  // fp_offset holds the offset in bytes from reg_save_area to the
  // place where the next available floating point argument register is saved.
  println("  movl $%d, 4(%%rax)", 48 + fp * 8);

  // Initialize reg_save_area.
  // reg_save_area points to the start of the register save area.
  println("  mov %%rbp, 16(%%rax)");
  println("  subq $128 ,16(%%rax)");
  top++;
}

static void gen_expr(Node *node)
{
  println("  .loc %d %d", node->tok->file_no, node->tok->line_no);
  switch (node->kind)
  {
  case ND_NUM:
    if (node->ty->kind == TY_FLOAT)
    {
      float fval_float = node->fval;
      println("  mov $%u, %%eax", *(unsigned *)(&fval_float));
      println("  movd %%eax, %%%s", freg(top++));
    }
    else if (node->ty->kind == TY_DOUBLE)
    {
      println("  mov $%lu, %%rax", *(unsigned long *)(&(node->fval)));
      println("  movq %%rax, %%%s", freg(top++));
    }
    else
      println("# gen_expr() ND_NUM");
      println("  li %s, %lu", reg(top++), node->val);
    return;
  case ND_VAR:
  case ND_MEMBER:
    println("# gen_expr() ND_MEMBER start");
    gen_addr(node);
    load(node->ty);
    println("# gen_expr() ND_MEMBER end");
    return;
  case ND_ASSIGN:
    if (node->ty->kind == TY_ARR)
      error_tok(node->tok, "not an lvalue");
    if (node->lhs->ty->is_const && !node->is_init)
      error_tok(node->tok, "cannnot assign to a const variable");
    gen_expr(node->rhs);
    gen_lval(node->lhs);
    store(node->ty);
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_DEREF:
    println("# ND_DEREF");
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
    cmp_zero(node->cond->ty);
    println("  je  .L.else.%d", seq);
    gen_expr(node->then);
    top--;
    println("  jmp .L.end.%d", seq);
    println(".L.else.%d:", seq);
    gen_expr(node->els);
    println(".L.end.%d:", seq);
    return;
  }
  case ND_NOT:
    println("# ND_NOT");
    gen_expr(node->lhs);
    cmp_zero(node->lhs->ty);
    println("  sete %%%sb", reg(top));
    println("  movzx %%%sb, %%%s", reg(top), reg(top));
    top++;
    return;
  case ND_BITNOT:
    gen_expr(node->lhs);
    println("  not %%%s", reg(top - 1));
    return;
  case ND_LOGAND:
  {
    int seq = labelseq++;
    gen_expr(node->lhs);
    cmp_zero(node->lhs->ty);
    println("  je  .L.false.%d", seq);
    gen_expr(node->rhs);
    cmp_zero(node->rhs->ty);
    println("  je  .L.false.%d", seq);
    println("  mov $1, %%%s", reg(top));
    println("  jmp .L.end.%d", seq);
    println(".L.false.%d:", seq);
    println("  mov $0, %%%s", reg(top++));
    println("  .L.end.%d:", seq);
    return;
  }
  case ND_LOGOR:
  {
    int seq = labelseq++;
    gen_expr(node->lhs);
    cmp_zero(node->lhs->ty);
    println("  jne .L.true.%d", seq);
    gen_expr(node->rhs);
    cmp_zero(node->rhs->ty);
    println("  jne .L.true.%d", seq);
    println("  mov $0, %%%s", reg(top));
    println("  jmp .L.end.%d", seq);
    println(".L.true.%d:", seq);
    println("  mov $1, %%%s", reg(top++));
    println("  .L.end.%d:", seq);
    return;
  }
  case ND_FUNCALL:
  {
    println("# ND_FUNCALL");
    if (node->lhs->kind == ND_VAR &&
        !strcmp(node->lhs->var->name, "__builtin_va_start"))
    {
      builtin_va_start(node);
      return;
    }

    // So far we only support up to 6 arguments.
    //
    // We should push ra becouse they are caller saved registers.
    println("  addi sp, sp, -8");
    println("  sd ra, (sp)");


    gen_expr(node->lhs);

    // Load arguments from the stack.

    // Index of argreg and fargreg.
    // Named according to "System V Application Binary Interface".
    // https://www.uclibc.org/docs/psABI-x86_64.pdf
    // g stands for general purpose registers.
    // f stands for floating point registers
    int gp = 0, fp = 0;

    for (int i = 0; i < node->nargs; i++)
    {
      Var *arg = node->args[i];

      if (is_flonum(arg->ty))
      {
        if (arg->ty->kind == TY_FLOAT)
          println("  movss -%d(%%rbp), %%%s", arg->offset, fargreg[fp++]);
        else
          println("  movsd -%d(%%rbp), %%%s", arg->offset, fargreg[fp++]);
        continue;
      }

      char *movop = arg->ty->is_unsigned ? "movz" : "movs";
      int sz = size_of(arg->ty);
      if (sz == 1)
        gen_offset_instr("lb", argreg[gp++], "s0", -1 * arg->offset);
      else if (sz == 2)
        gen_offset_instr("lh", argreg[gp++], "s0", -1 * arg->offset);
      else if (sz == 4)
        gen_offset_instr("lw", argreg[gp++], "s0", -1 * arg->offset);
      else
        gen_offset_instr("ld", argreg[gp++], "s0", -1 * arg->offset);
    }
    // // Set the number of vector registers used to rax
    // println("  mov $%d, %%rax", fp);
    
    println("  jalr %s", reg(--top));

    // The System V x86-64 ABI has a special rule regarding a boolean return
    // value that onlyu the lower 8 bits are valid for it and the upper
    // 56 bits may contain garbage. Here, we clear the upper 56 bits.
    if (node->ty->kind == TY_BOOL)
      println("  movzx %%al, %%rax");


    // Restore caaller-saved registers
    println("  ld ra, (sp)");
    println("  addi sp, sp, 8");

    if (node->ty->kind == TY_FLOAT)
      println("  movss %%xmm0, %%%s", freg(top++));
    else if (node->ty->kind == TY_DOUBLE)
      println("  movsd %%xmm0, %%%s", freg(top++));
    else
      println("  mv %s, a0", reg(top++));
    
    return;
  }
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

  char *rd = xreg(node->lhs->ty, top - 2);
  char *rs = xreg(node->lhs->ty, top - 1);
  char *fd = freg(top - 2);
  char *fs = freg(top - 1);
  top--;

  switch (node->kind)
  {
  case ND_ADD:
    if (node->ty->kind == TY_FLOAT)
      println("  addss %%%s, %%%s", fs, fd);
    else if (node->ty->kind == TY_DOUBLE)
      println("  addsd %%%s, %%%s", fs, fd);
    else
      println("  add %s, %s, %s", rd, rd, rs);
    break;
  case ND_PTR_ADD:
    println("  li t1, %d", node->ty->base->size);
    println("  mul %s, %s, t1", rs, rs);
    println("  add %s, %s, %s", rd, rd, rs);
    break;
  case ND_SUB:
    if (node->ty->kind == TY_FLOAT)
      println("  subss %%%s, %%%s", fs, fd);
    else if (node->ty->kind == TY_DOUBLE)
      println("  subsd %%%s, %%%s", fs, fd);
    else
      println("  sub %s, %s, %s", rd, rd, rs);
    break;
  case ND_PTR_SUB:
    println("  li t1, %d", node->ty->base->size);
    println("  mul %s, %s, t1", rs, rs);
    println("  sub %s, %s, %s", rd, rd, rs);
    break;
  case ND_PTR_DIFF:
    println("  sub %s, %s, %s", rd, rd, rs);
    println("  li t1, %d", node->lhs->ty->base->size);
    println("  divu %s, %s, t1", rd, rd);
    break;
  case ND_MUL:
    if (node->ty->kind == TY_FLOAT)
      println("  mulss %%%s, %%%s", fs, fd);
    else if (node->ty->kind == TY_DOUBLE)
      println("  mulsd %%%s, %%%s", fs, fd);
    else
      println("  mul %s, %s, %s", rd, rd, rs);
    break;
  case ND_DIV:
    if (node->ty->kind == TY_FLOAT)
      println("  divss %%%s, %%%s", fs, fd);
    else if (node->ty->kind == TY_DOUBLE)
      println("  divsd %%%s, %%%s", fs, fd);
    else
    {
      if (node->ty->is_unsigned)
        println("  divu %s, %s, %s", rd, rd, rs);
      else
        println("  div %s, %s, %s", rd, rd, rs);
    }
    break;
  case ND_MOD:
    divmod(node, rd, rs, "rdx", "edx");
    return;
  case ND_BITAND:
    println("  and %%%s, %%%s", rs, rd);
    return;
  case ND_BITOR:
    println("  or %%%s, %%%s", rs, rd);
    return;
  case ND_BITXOR:
    println("  xor %%%s, %%%s", rs, rd);
    return;
  case ND_EQ:
    if (node->lhs->ty->kind == TY_FLOAT)
      println("  ucomiss %%%s, %%%s", fs, fd);
    else if (node->lhs->ty->kind == TY_DOUBLE)
      println("  ucomisd %%%s, %%%s", fs, fd);
    else
    {
      println("  sub %s, %s, %s", rd, rd, rs);
      println("  seqz %s, %s", rd, rd);
    }
    // println("  sete %%al");
    // println("  movzb %%al, %%%s", rd);
    break;
  case ND_NE:
    if (node->lhs->ty->kind == TY_FLOAT)
      println("  ucomiss %%%s, %%%s", fs, fd);
    else if (node->lhs->ty->kind == TY_DOUBLE)
      println("  ucomisd %%%s, %%%s", fs, fd);
    else
    {
      println("  sub %s, %s, %s", rd, rd, rs);
      println("  snez %s, %s", rd, rd);
    }
    // println("  setne %%al");
    // println("  movzb %%al, %%%s", rd);
    break;
  case ND_LT:
    if (node->lhs->ty->kind == TY_FLOAT)
    {
      println("  ucomiss %%%s, %%%s", fs, fd);
      println("  setb %%al");
    }
    else if (node->lhs->ty->kind == TY_DOUBLE)
    {
      println("  ucomisd %%%s, %%%s", fs, fd);
      println("  setb %%al");
    }
    else
    {
      if (node->lhs->ty->is_unsigned)
        println("  sltu %s, %s, %s", rd, rd, rs);
      else
        println("  slt %s, %s, %s", rd, rd, rs);
    }
    // println("  movzb %%al, %%%s", rd);
    break;
  case ND_LE:
    if (node->lhs->ty->kind == TY_FLOAT)
    {
      println("  ucomiss %%%s, %%%s", fs, fd);
      println("  setbe %%al");
    }
    else if (node->lhs->ty->kind == TY_DOUBLE)
    {
      println("  ucomisd %%%s, %%%s", fs, fd);
      println("  setbe %%al");
    }
    else
    {
      if (node->lhs->ty->is_unsigned)
        println("  sltu %s, %s, %s", rd, rs, rd);
      else
        println("  slt %s, %s, %s", rd, rs, rd);
      println("  seqz %s, %s", rd, rd);
    }
    // println("  movzb %%al, %%%s", rd);
    break;
  case ND_SHL:
    println("  mov %%%s, %%rcx", reg(top));
    println("  shl %%cl, %%%s", rd);
    return;
  case ND_SHR:
    println("  mov %%%s, %%rcx", reg(top));
    if (node->lhs->ty->is_unsigned)
      println("  shr %%cl, %%%s", rd);
    else
      println("  sar %%cl, %%%s", rd);
    return;
  default:
    error_tok(node->tok, "invalid statement");
  }
}

static void gen_stmt(Node *node)
{
  println("  .loc %d %d", node->tok->file_no, node->tok->line_no);

  switch (node->kind)
  {
  case ND_IF:
  {
    int seq = labelseq++;
    if (node->els)
    {
      gen_expr(node->cond);
      cmp_zero(node->cond->ty);
      println("  bne %s, zero, .L.else.%d", reg(top), seq);
      gen_stmt(node->then);
      println("  jal zero, .L.end.%d", seq);
      println(".L.else.%d:", seq);
      gen_stmt(node->els);
      println(".L.end.%d:", seq);
    }
    else
    {
      gen_expr(node->cond);
      cmp_zero(node->cond->ty);
      println("  bne %s, zero, .L.end.%d", reg(top), seq);
      gen_stmt(node->then);
      println(".L.end.%d:", seq);
    }
    return;
  }
  case ND_WHILE:
  {
    int seq = labelseq++;
    int brk = brkseq;
    int cont = contseq;
    brkseq = contseq = seq;

    println(".L.begin.%d:", seq);
    gen_expr(node->cond);
    println("  beq %s, zero, .L.break.%d", reg(--top), seq);
    gen_stmt(node->then);
    println(".L.continue.%d:", seq);
    println("  jal zero, .L.begin.%d", seq);
    println(".L.break.%d:", seq);

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

    println(".L.begin.%d:", seq);
    gen_stmt(node->then);
    println(".L.continue.%d:", seq);
    gen_expr(node->cond);
    cmp_zero(node->cond->ty);
    println("  jne .L.begin.%d", seq);
    println(".L.break.%d:", seq);

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

    println("# for init start");
    if (node->init)
      gen_stmt(node->init);
    println("# for init end");
    println(".L.begin.%d:", seq);
    if (node->cond)
    {
      println("# for cond start");
      gen_expr(node->cond);
      println("# for cond end");
      // char *rd = reg(--top);
      // char *rs = rd;
      // println("  seqz %s, %s", rd, rs);
      // println("  bne %s, zero, .L.break.%d", rd, seq);
      println("  beq %s, zero, .L.break.%d", reg(--top), seq);
    }
    println("# for then start");
    gen_stmt(node->then);
    println("# for then end");
    println(".L.continue.%d:", seq);
    println("# for inc start");
    if (node->inc)
      gen_stmt(node->inc);
    println("# for inc end");
    println("  jal zero, .L.begin.%d", seq);
    println(".L.break.%d:", seq);

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
      println("  cmp $%ld, %%%s", n->val, reg(top - 1));
      println("  je .L.case.%d", n->case_label);
    }
    top--;

    if (node->default_case)
    {
      int i = labelseq++;
      node->default_case->case_label = i;
      println("  jmp .L.case.%d", i);
    }

    println("  jmp .L.break.%d", seq);
    gen_stmt(node->then);
    println(".L.break.%d:", seq);

    brkseq = brk;
    return;
  }
  case ND_CASE:
    println(".L.case.%d:", node->case_label);
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
    println("  jmp .L.break.%d", brkseq);
    return;
  case ND_CONTINUE:
    if (contseq == 0)
      error_tok(node->tok, "stray continue");
    println("  jmp .L.continue.%d", contseq);
    return;
  case ND_GOTO:
    println("  jmp .L.label.%s.%s", current_fn->name, node->label_name);
    return;
  case ND_LABEL:
    println(".L.label.%s.%s:", current_fn->name, node->label_name);
    gen_stmt(node->lhs);
    return;
  case ND_RETURN:
    if (node->lhs)
    {
      gen_expr(node->lhs);
      if (is_flonum(node->lhs->ty))
        println("  movsd %%%s, %%xmm0", freg(--top));
      else
      {
        println("# gen_stmt() !is_flonum(node->lhs->ty)");
        println("  mv a0, %s", reg(--top));
      }
    }
    println("  j .L.return.%s", current_fn->name);
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
  println(".bss");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;

    if (var->init_data)
      continue;

    println("  .align %d", var->align);
    if (!var->is_static)
      println("  .globl %s", var->name);
    println("%s:", var->name);
    println("  .zero %d", size_of(var->ty));
  }
}

static void emit_data(Program *prog)
{
  println(".data");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;
    if (!var->init_data)
      continue;

    println("  .align %d", var->align);
    if (!var->is_static)
      println("  .globl %s", var->name);
    println("%s:", var->name);
    Relocation *rel = var->rel;
    int pos = 0;
    while (pos < var->ty->size)
    {
      if (rel && rel->offset == pos)
      {
        println("  .quad %s%+ld", rel->label, rel->addend);
        rel = rel->next;
        pos += 8;
      }
      else
        println("  .byte %d", var->init_data[pos++]);
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
  println(".text");
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    current_fn = fn;
    if (!fn->is_static)
      println(".global %s", fn->name);
    println("%s:", fn->name);

    // Prologue. s0 ~ s11 are callee-saved registers.
    // For frame pointer
    println("  addi sp, sp, -8");
    // Save frame pointer
    println("  sd s0, (sp)");

    println("  mv s0, sp");
    gen_addi("sp", "sp", -1 * fn->stack_size);
    // println("  sd s0, -8(s0)");
    println("  sd s1, -16(s0)");
    println("  sd s2, -24(s0)");
    println("  sd s3, -32(s0)");
    println("  sd s4, -40(s0)");
    println("  sd s5, -48(s0)");
    println("  sd s6, -56(s0)");
    println("  sd s7, -64(s0)");
    println("  sd s8, -72(s0)");
    println("  sd s9, -80(s0)");
    println("  sd s10, -88(s0)");
    println("  sd s11, -96(s0)");

    // Save arg registers to the register save area
    // if the function is the variadic
    if (fn->is_variadic)
    {
      println("  mov %%rdi, -128(%%rbp)");
      println("  mov %%rsi, -120(%%rbp)");
      println("  mov %%rdx, -112(%%rbp)");
      println("  mov %%rcx, -104(%%rbp)");
      println("  mov %%r8, -96(%%rbp)");
      println("  mov %%r9, -88(%%rbp)");

      println("  movsd %%xmm0, -80(%%rbp)");
      println("  movsd %%xmm1, -72(%%rbp)");
      println("  movsd %%xmm2, -64(%%rbp)");
      println("  movsd %%xmm3, -56(%%rbp)");
      println("  movsd %%xmm4, -48(%%rbp)");
      println("  movsd %%xmm5, -40(%%rbp)");
    }

    // Save arguments to the stack

    // g stands for general purpose registers.
    // f stands for floating point registers
    int gp = 0, fp = 0;
    for (VarList *param = fn->params; param; param = param->next)
      if (is_flonum(param->var->ty))
        fp++;
      else
        gp++;

    for (VarList *param = fn->params; param; param = param->next)
    {
      Var *var = param->var;
      if (var->ty->kind == TY_FLOAT)
        println("  movss %%%s, -%d(%%rbp)", fargreg[--fp], var->offset);
      else if (var->ty->kind == TY_DOUBLE)
        println("  movsd %%%s, -%d(%%rbp)", fargreg[--fp], var->offset);
      else
      {
        int sz = size_of(var->ty);
        if (sz == 1)
          gen_offset_instr("sb", argreg[--gp], "s0", -1 * var->offset);
        else if (sz == 2)
          gen_offset_instr("sh", argreg[--gp], "s0", -1 * var->offset);
        else if (sz == 4)
          gen_offset_instr("sw", argreg[--gp], "s0", -1 * var->offset);
        else
          gen_offset_instr("sd", argreg[--gp], "s0", -1 * var->offset);
      }
    }

    // Generate statements
    for (Node *node = fn->node; node; node = node->next)
    {
      gen_stmt(node);
      assert(top == 0);
    }

    // Epilogue
    // Restore the values of sp, s0 ~ s11
    println(".L.return.%s:", fn->name);
  
    // println("  ld s0, -8(s0)");
    println("  ld s1, -16(s0)");
    println("  ld s2, -24(s0)");
    println("  ld s3, -32(s0)");
    println("  ld s4, -40(s0)");
    println("  ld s5, -48(s0)");
    println("  ld s6, -56(s0)");
    println("  ld s7, -64(s0)");
    println("  ld s8, -72(s0)");
    println("  ld s9, -80(s0)");
    println("  ld s10, -88(s0)");
    println("  ld s11, -96(s0)");
  
    println("  mv sp, s0");
    println("  ld s0, (sp)");
    println("  addi sp, sp, 8");
    println("  ret");
  }
}

void codegen(Program *prog)
{
  // Output the assembly code.

  char **paths = get_input_files();
  for (int i = 0; paths[i]; i++)
    println(".file %d \"%s\"", i + 1, paths[i]);

  emit_bss(prog);
  emit_data(prog);
  emit_text(prog);
}
