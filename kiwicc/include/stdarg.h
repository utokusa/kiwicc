#ifndef __STDARG_H
#define __STDARG_H

typedef struct
{
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} va_list[1];

typedef va_list __builtin_va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
typedef __builtin_va_list __gnuc_va_list;

#endif /* __STDARG_H */