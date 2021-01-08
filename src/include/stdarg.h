#ifndef __STDARG_H
#define __STDARG_H

typedef void * va_list;

typedef va_list __builtin_va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
typedef __builtin_va_list __gnuc_va_list;

#endif /* __STDARG_H */