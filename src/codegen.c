#include "kiwicc.h"

/*********************************************
* ...code generator...
*********************************************/

static int top;
static int labelseq = 1;
static int brkseq;
static int contseq;
static Function *current_fn;
static char *argreg[] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
static int reg_save_area_offset[] = {-248/*a0*/, -240/*a1*/, -232/*a2*/, -224/*a3*/,
                                     -216/*a4*/, -208/*a5*/, -200/*a6*/, -192/*a7*/};
static char *fargreg[] = {"fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7"};
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
  static char *r[] = {"fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11"};
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

// For `lb`, `lh`, `lw`, `ld`, `sb`, `sh`, `sw`, `sd`
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
    println("  flw %s, (%s)", freg(top - 1), reg(top - 1));
    return;
  }
  if (ty->kind == TY_DOUBLE)
  {
    println("  fld %s, (%s)", freg(top - 1), reg(top - 1));
    return;
  }

  char *rs = reg(top - 1);
  char *rd = xreg(ty, top - 1);
  char *suffix = ty->is_unsigned ? "u" : "";
  int sz = size_of(ty);

  if (sz == 1)
    println("  lb%s %s, (%s)", suffix, rd, rs);
  else if (sz == 2)
    println("  lh%s %s, (%s)", suffix, rd, rs);
  else if (sz == 4)
    println("  lw%s %s, (%s)", suffix, rd, rs);
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
    println("  fsw %s, (%s)", freg(top - 2), rd);
  else if (ty->kind == TY_DOUBLE)
    println("  fsd %s, (%s)", freg(top - 2), rd);
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
    char *fs = freg(--top);
    char *rd = reg(top);
    println("  fmv.s.x ft0, zero");
    println("  feq.s %s, %s, ft0", rd, fs);
  }
  else if (ty->kind == TY_DOUBLE)
  {
    char *fs = freg(--top);
    char *rd = reg(top);
    println("  fmv.d.x ft0, zero");
    println("  feq.d %s, %s, ft0", rd, fs);
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
    println("  seqz %s, %s", reg(top), reg(top));
    println("  andi %s, %s, 0xff", reg(top), reg(top));
    top++;
    return;
  }

  if (from->kind == TY_FLOAT)
  {
    if (to->kind == TY_FLOAT)
      return;
    
    if (to->kind == TY_DOUBLE)
      println("  fcvt.d.s %s, %s", fr, fr);
    else /* integer */
      println("  fcvt.l.s %s, %s, rtz", r, fr);
    return;
  }

  if (from->kind == TY_DOUBLE)
  {
    if (to->kind == TY_DOUBLE)
      return;
    
    if (to->kind == TY_FLOAT)
      println("  fcvt.s.d %s, %s", fr, fr);
    else /* integer */
      println("  fcvt.l.d %s, %s, rtz", r, fr);
    return;
  }

  if (to->kind == TY_FLOAT)
  {
    println("  fcvt.s.l %s, %s", fr, r);
    return;
  }

  if (to->kind == TY_DOUBLE)
  {
    println("  fcvt.d.l %s, %s", fr, r);
    return;
  }

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
    
    // TODO: handle "-fpic" option
    println("  la %s, %s", reg(top++), var->name);

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

// Initialize va_list.
static void builtin_va_start(Node *node)
{
  println("# builtin_va_start");
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

  // Initializes va_list argument to point to the start of the vararg save area
  println("  addi t1, s0, %d", reg_save_area_offset[gp]);

  // The offset for va_list from s0 is node->args[0]->offset + 8.
  // `+8` is for ra saved in stack
  println("  sd t1, -%d(s0)", node->args[0]->offset + 8);
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
      println("  li t1, %lu", *(unsigned *)(&fval_float));
      println("  addi sp, sp, -8");
      println("  sw t1, (sp)");
      println("  flw %s, (sp)", freg(top++));
      println("  addi sp, sp, 8");
    }
    else if (node->ty->kind == TY_DOUBLE)
    {
      println("  li t1, %lu", *(unsigned long *)(&(node->fval)));
      println("  addi sp, sp, -8");
      println("  sd t1, (sp)");
      println("  fld %s, (sp)", freg(top++));
      println("  addi sp, sp, 8");
    }
    else
    {
      println("# gen_expr() ND_NUM");
      println("  li %s, %ld", reg(top++), node->val);
    }
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
    println("  bne %s, zero, .L.else.%d", reg(top), seq);
    gen_expr(node->then);
    top--;
    println("  j .L.end.%d", seq);
    println(".L.else.%d:", seq);
    gen_expr(node->els);
    println(".L.end.%d:", seq);
    return;
  }
  case ND_NOT:
    println("# ND_NOT");
    gen_expr(node->lhs);
    println("# checing %s value", reg(top - 1));
    cmp_zero(node->lhs->ty);
    println("  snez %s, %s", reg(top), reg(top));
    println("  andi %s, %s, 0xff", reg(top), reg(top));
    top++;
    return;
  case ND_BITNOT:
    gen_expr(node->lhs);
    println("  not %s, %s", reg(top - 1), reg(top - 1));
    return;
  case ND_LOGAND:
  {
    int seq = labelseq++;
    gen_expr(node->lhs);
    cmp_zero(node->lhs->ty);
    println("  bne %s, zero, .L.false.%d", reg(top), seq);
    gen_expr(node->rhs);
    cmp_zero(node->rhs->ty);
    println("  bne %s, zero, .L.false.%d", reg(top), seq);
    println("  li %s, 1", reg(top));
    println("  j .L.end.%d", seq);
    println(".L.false.%d:", seq);
    println("  mv %s, zero", reg(top++));
    println("  .L.end.%d:", seq);
    return;
  }
  case ND_LOGOR:
  {
    int seq = labelseq++;
    gen_expr(node->lhs);
    cmp_zero(node->lhs->ty);
    println("  beq %s, zero, .L.true.%d", reg(top), seq);
    gen_expr(node->rhs);
    cmp_zero(node->rhs->ty);
    println("  beq %s, zero, .L.true.%d", reg(top), seq);
    println("  mv %s, zero", reg(top));
    println("  j .L.end.%d", seq);
    println(".L.true.%d:", seq);
    println("  li %s, 1", reg(top++));
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

    // We should push ra becouse they are caller saved registers.
    println("  addi sp, sp, -8");
    println("  sd ra, (sp)");


    gen_expr(node->lhs);

    // Load arguments from the stack.

    // Index of argreg and fargreg.
    // g stands for general purpose registers.
    // f stands for floating point registers
    int gp = 0, fp = 0;

    for (int i = 0; i < node->nargs; i++)
    {
      Var *arg = node->args[i];

      if (is_flonum(arg->ty))
      {
        if (arg->ty->kind == TY_FLOAT)
        {
          gen_offset_instr("flw", fargreg[fp++], "s0", -1 * arg->offset);
          println("  fmv.x.w  %s, %s", argreg[gp++], fargreg[fp - 1]);
        }
        else
        {
          gen_offset_instr("fld", fargreg[fp++], "s0", -1 * arg->offset);
          println("  fmv.x.d  %s, %s", argreg[gp++], fargreg[fp - 1]);
        }
        continue;
      }

      char *suffix = arg->ty->is_unsigned ? "u" : "";
      char instr[4];
      int sz = size_of(arg->ty);
      if (sz == 1)
        sprintf(instr, "lb%s", suffix);
      else if (sz == 2)
        sprintf(instr, "lh%s", suffix);
      else if (sz == 4)
        sprintf(instr, "lw%s", suffix);
      else
        sprintf(instr, "ld");
      gen_offset_instr(instr, argreg[gp++], "s0", -1 * arg->offset);
    }
    
    println("  jalr %s", reg(--top));

    // If return type is boolean, only the lower 8 bits are valid for it and the upper
    // 56 bits may contain garbage. Here, we clear the upper 56 bits.
    if (node->ty->kind == TY_BOOL)
      println("  andi a0, a0, 0xff");


    // Restore caaller-saved registers
    println("  ld ra, (sp)");
    println("  addi sp, sp, 8");

    if (node->ty->kind == TY_FLOAT)
      println("  fmv.s %s, fa0", freg(top++));
    else if (node->ty->kind == TY_DOUBLE)
      println("  fmv.d %s, fa0", freg(top++));
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
      println("  fadd.s %s, %s, %s", fd, fd, fs);
    else if (node->ty->kind == TY_DOUBLE)
      println("  fadd.d %s, %s, %s", fd, fd, fs);
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
      println("  fsub.s %s, %s, %s", fd, fd, fs);
    else if (node->ty->kind == TY_DOUBLE)
      println("  fsub.d %s, %s, %s", fd, fd, fs);
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
      println("  fmul.s %s, %s, %s", fd, fd, fs);
    else if (node->ty->kind == TY_DOUBLE)
      println("  fmul.d %s, %s, %s", fd, fd, fs);
    else
      println("  mul %s, %s, %s", rd, rd, rs);
    break;
  case ND_DIV:
    if (node->ty->kind == TY_FLOAT)
      println("  fdiv.s %s, %s, %s", fd, fd, fs);
    else if (node->ty->kind == TY_DOUBLE)
      println("  fdiv.d %s, %s, %s", fd, fd, fs);
    else
    {
      if (node->ty->is_unsigned)
        println("  divu %s, %s, %s", rd, rd, rs);
      else
        println("  div %s, %s, %s", rd, rd, rs);
    }
    break;
  case ND_MOD:
    if (node->ty->is_unsigned)
      println("  rem %s, %s, %s", rd, rd, rs);
    else
      println("  remu %s, %s, %s", rd, rd, rs);
    return;
  case ND_BITAND:
    println("  and %s, %s, %s", rd, rd, rs);
    return;
  case ND_BITOR:
    println("  or %s, %s, %s", rd, rd, rs);
    return;
  case ND_BITXOR:
    println("  xor %s, %s, %s", rd, rd, rs);
    return;
  case ND_EQ:
    if (node->lhs->ty->kind == TY_FLOAT)
      println("  feq.s %s, %s, %s", rd, fd, fs);
    else if (node->lhs->ty->kind == TY_DOUBLE)
      println("  feq.d %s, %s, %s", rd, fd, fs);
    else
    {
      println("  sub %s, %s, %s", rd, rd, rs);
      println("  seqz %s, %s", rd, rd);
    }
    break;
  case ND_NE:
    if (node->lhs->ty->kind == TY_FLOAT)
    {
      println("  feq.s %s, %s, %s", rd, fd, fs);
      println("  seqz %s, %s", rd, rd);
    }
    else if (node->lhs->ty->kind == TY_DOUBLE)
    {
      println("  feq.d %s, %s, %s", rd, fd, fs);
      println("  seqz %s, %s", rd, rd);
    }
    else
    {
      println("  sub %s, %s, %s", rd, rd, rs);
      println("  snez %s, %s", rd, rd);
    }
    break;
  case ND_LT:
    if (node->lhs->ty->kind == TY_FLOAT)
      println("  flt.s %s, %s, %s", rd, fd, fs);
    else if (node->lhs->ty->kind == TY_DOUBLE)
      println("  flt.d %s, %s, %s", rd, fd, fs);
    else
    {
      if (node->lhs->ty->is_unsigned)
        println("  sltu %s, %s, %s", rd, rd, rs);
      else
        println("  slt %s, %s, %s", rd, rd, rs);
    }
    break;
  case ND_LE:
    if (node->lhs->ty->kind == TY_FLOAT)
      println("  fle.s %s, %s, %s", rd, fd, fs);
    else if (node->lhs->ty->kind == TY_DOUBLE)
      println("  fle.d %s, %s, %s", rd, fd, fs);
    else
    {
      if (node->lhs->ty->is_unsigned)
        println("  sltu %s, %s, %s", rd, rs, rd);
      else
        println("  slt %s, %s, %s", rd, rs, rd);
      println("  seqz %s, %s", rd, rd);
    }
    break;
  case ND_SHL:
    println("  sll %s, %s, %s", rd, rd, rs);
    return;
  case ND_SHR:
    if (node->lhs->ty->is_unsigned)
      println("  srl %s, %s, %s", rd, rd, rs);
    else
      println("  sra %s, %s, %s", rd, rd, rs);
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
    println("  beq %s, zero, .L.begin.%d", reg(top), seq);
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
      println("  li a0, %d", n->val);
      println("  beq a0, %s, .L.case.%d", reg(top - 1), n->case_label);
    }
    top--;

    if (node->default_case)
    {
      int i = labelseq++;
      node->default_case->case_label = i;
      println("  j .L.case.%d", i);
    }

    println("  j .L.break.%d", seq);
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
    println("  j .L.break.%d", brkseq);
    return;
  case ND_CONTINUE:
    if (contseq == 0)
      error_tok(node->tok, "stray continue");
    println("  j .L.continue.%d", contseq);
    return;
  case ND_GOTO:
    println("  j .L.label.%s.%s", current_fn->name, node->label_name);
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
        println("  fmv.d fa0, %s", freg(--top));
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

static int int_log2(int x) {
  int i = 0;
  int v = 1;
  while (v < x)
  {
    v = v << 1;
    i++;
  }
  return (v == x) ? i : -1;
}

static void emit_bss(Program *prog)
{
  println(".bss");
  for (VarList *vl = prog->globals; vl; vl = vl->next)
  {
    Var *var = vl->var;

    if (var->init_data)
      continue;

    int align = int_log2(var->align);
    if (align == -1)
      error_tok(var->tok, "requested alignment is not a positive power of 2");
    println("  .align %d", align);
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

    int align = int_log2(var->align);
    if (align == -1)
      error_tok(var->tok, "requested alignment is not a positive power of 2");
    println("  .align %d", align);
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

static void emit_text(Program *prog)
{
  println(".text");
  for (Function *fn = prog->fns; fn; fn = fn->next)
  {
    current_fn = fn;
    if (!fn->is_static)
      println(".global %s", fn->name);
    println("%s:", fn->name);

    // Prologue. s0 ~ s11 and fs0 ~ fs11 are callee-saved registers.
    // For frame pointer
    println("  addi sp, sp, -8");
    // Save frame pointer
    println("  sd s0, (sp)");

    println("  mv s0, sp");
    gen_addi("sp", "sp", -1 * fn->stack_size);
    println("  sd s1, -8(s0)");
    println("  sd s2, -16(s0)");
    println("  sd s3, -24(s0)");
    println("  sd s4, -32(s0)");
    println("  sd s5, -40(s0)");
    println("  sd s6, -48(s0)");
    println("  sd s7, -56(s0)");
    println("  sd s8, -64(s0)");
    println("  sd s9, -72(s0)");
    println("  sd s10, -80(s0)");
    println("  sd s11, -88(s0)");

    println("  fsd fs0, -96(s0)");
    println("  fsd fs1, -104(s0)");
    println("  fsd fs2, -112(s0)");
    println("  fsd fs3, -120(s0)");
    println("  fsd fs4, -128(s0)");
    println("  fsd fs5, -136(s0)");
    println("  fsd fs6, -144(s0)");
    println("  fsd fs7, -152(s0)");
    println("  fsd fs8, -160(s0)");
    println("  fsd fs9, -168(s0)");
    println("  fsd fs10, -176(s0)");
    println("  fsd fs11, -184(s0)");

    // Save arg registers to the register save area
    // if the function is the variadic
    if (fn->is_variadic)
    {
      println("  sd a0, %d(s0)", reg_save_area_offset[0]);
      println("  sd a1, %d(s0)", reg_save_area_offset[1]);
      println("  sd a2, %d(s0)", reg_save_area_offset[2]);
      println("  sd a3, %d(s0)", reg_save_area_offset[3]);
      println("  sd a4, %d(s0)", reg_save_area_offset[4]);
      println("  sd a5, %d(s0)", reg_save_area_offset[5]);
      println("  sd a6, %d(s0)", reg_save_area_offset[6]);
      println("  sd a7, %d(s0)", reg_save_area_offset[7]);
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
        gen_offset_instr("fsw", fargreg[--fp], "s0", -1 * var->offset);
      else if (var->ty->kind == TY_DOUBLE)
        gen_offset_instr("fsd", fargreg[--fp], "s0", -1 * var->offset);
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
  
    println("  ld s1, -8(s0)");
    println("  ld s2, -16(s0)");
    println("  ld s3, -24(s0)");
    println("  ld s4, -32(s0)");
    println("  ld s5, -40(s0)");
    println("  ld s6, -48(s0)");
    println("  ld s7, -56(s0)");
    println("  ld s8, -64(s0)");
    println("  ld s9, -72(s0)");
    println("  ld s10, -80(s0)");
    println("  ld s11, -88(s0)");

    println("  fld fs0, -96(s0)");
    println("  fld fs1, -104(s0)");
    println("  fld fs2, -112(s0)");
    println("  fld fs3, -120(s0)");
    println("  fld fs4, -128(s0)");
    println("  fld fs5, -136(s0)");
    println("  fld fs6, -144(s0)");
    println("  fld fs7, -152(s0)");
    println("  fld fs8, -160(s0)");
    println("  fld fs9, -168(s0)");
    println("  fld fs10, -176(s0)");
    println("  fld fs11, -184(s0)");
  
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
