// This is a line comment.

/*
 * This is a block comment.
 */

// Null directive
#
#// comment
#/* comment */

int printf();
int exit();
int strcmp(char *p, char *q);
int memcmp(char *, char *, long);

int ret_777();

int g1;
int g2[4];
char g3;
char g4[4];

char g5 = 5;
short g6 = 6;
int g7 = 7;
long g8 = 8;

char g9[] = "abc";
int g10[3] = {0, 1, 2};
struct
{
  char a;
  int b;
} g11[2] = {{1, 2}, {3, 4}};

struct
{
  int a[2];
} g12[2] = {{{1, 2}}};

char g13[] = "foobar";
char g14[10] = "foobar";
char g15[3] = "foobar";
char *g16 = g13 + 3;
char *g17 = g13 + 0;
char *g18 = &g13 - 3;
char *g19[] = {g13 + 0, g13 + 3, g13 - 3};
int g20 = 5;
int *g21 = &g20;
int g22[] = {2, 4, 6};
int *g23 = g22 + 1;
int **g24 = &g23;
int *g25 = &g11[1].a;
int g26[2][2] = {{1, 2}, {3, 4}};
int *g27 = &g26[1][1];
int *g28[2] = {&g26[0][1], &g26[1][0]};
struct
{
  int a[2];
} g29[2] = {1, 2, 3, 4};
struct
{
  int a[2];
} g30[2] = {1, 2, 3, 4};
char g31[][4] = {'f', 'o', 'o', 0, 'b', 'a', 'r', 0};
char *g32 = {"foobar"};

// g33, ret7()
#include "include.h"
#define M1 2020
#define MY_INT int
#define RET3 ret3()

typedef struct Tree
{
  int val;
  struct Tree *lhs;
  struct Tree *rhs;
} Tree;

Tree *tree = &(Tree){1, &(Tree){2, &(Tree){3, 0, 0}, &(Tree){4, 0, 0}}, 0};

extern int ext1;
extern int *ext2;
static int ext3 = 3;

int;
struct
{
  char a;
  char b;
};
typedef struct
{
  char a;
  int b;
} Ty1;

typedef int MyInt, MyInt2[4];

int _Alignas(512) g_aligned1;
int _Alignas(512) g_aligned2;
int _Alignas(128) g_aligned3;

int assert(long expected, long actual, char *code)
{
  if (expected == actual)
  {
    printf("%s => %ld\n", code, actual);
  }
  else
  {
    printf("%s => %ld expected but got %ld\n", code, expected, actual);
    exit(1);
  }
}

int ret3() { return 3; }
int ret5(void) { return 5; }
int return_1() { return 1; }
int add(int x, int y) { return x + y; }
int sub(int x, int y) { return x - y; }
int add5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }
int retx(int x) { return x; }
int fast_return()
{
  return 10;
  return 20;
}
int use_if()
{
  if (1)
    return 1;
  return 2;
}
int ret_0()
{
  for (;;)
    return 0;
}
int ret_10()
{
  int a;
  a = 0;
  for (;; a = a + 1)
    if (a >= 10)
      return a;
}
int fib(int x)
{
  if (x <= 1)
    return 1;
  return fib(x - 1) + fib(x - 2);
}
int sum_arr_elems(int *a) { return a[0] + a[1] + a[2]; }
int add_char1(char a, char b, char c) { return a + b + c; }
char add_char2(char a, char b, char c) { return a + b + c; }
int add6(int a, int b, int c, int d, int e, int f)
{
  return a + b + c + d + e + f;
}
int sub_short(short a, short b, short c)
{
  return a - b - c;
}
int sub_long(long a, long b, long c)
{
  return a - b - c;
}
int *g1_ptr()
{
  return &g1;
}
char int_to_char(int x)
{
  return x;
}
int int_to_int(int x)
{
  return x;
}
int div_long(long a, long b)
{
  return a / b;
}
_Bool bool_fn_add(_Bool x)
{
  return x + 1;
}
_Bool bool_fn_sub(_Bool x)
{
  return x - 1;
}
static int static_fn()
{
  return 3;
}
int param_decay(int x[])
{
  return x[0];
}
int counter()
{
  static int i;
  static int a = 5;
  return i++ + a;
}
int ret_none()
{
  3;
  return;
}

int const_arg_func(const int x) 
{
  return x;
}

const int const_rest_func(int x) 
{
  return x;
}

_Bool true_fn();
_Bool false_fn();

_Noreturn noreturn_fn()
{
  exit(0);
}

typedef struct
{
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} va_list[1];

int add_all1(int x, ...);
int add_all3(int x, int y, int z, ...);
int sprintf(char *buf, char *fmt, ...);
int vsprintf(char *buf, char *fmt, ...);

char *fmt(char *buf, char *fmt, ...)
{
  va_list ap;
  __builtin_va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
}

int main()
{
  assert(0, 0, "0");
  assert(0, ({ 0; }), "({ 0; })");
  assert(42, ({ 42; }), "({ 42; })");
  assert(21, ({ 5 + 20 - 4; }), "({ 5 + 20 - 4; })");
  assert(5, ({ 1 + 1 + 1 + 1 + 1; }), "({ 1 + 1 + 1 + 1 + 1; })");
  assert(0, ({ 1 + 1 + 1 + 1 + 1 - 1 - 1 - 1 - 1 - 1; }), "({ 1 + 1 + 1 + 1 + 1 - 1 - 1 - 1 - 1 - 1; })");
  assert(41, ({ 12 + 34 - 5; }), "({ 12 + 34 - 5; })");
  assert(47, ({ 5 + 6 * 7; }), "({ 5 + 6 * 7; })");
  assert(15, ({ 5 * (9 - 6); }), "({ 5 * (9 - 6); })");
  assert(4, ({ (3 + 5) / 2; }), "({ (3 + 5) / 2; })");
  assert(8, ({ -5 + 5 * 3 - 2 * 1; }), "({ -5 + 5 * 3 - 2 * 1; })");

  assert(2, ({ +1 + 1; }), "({ +1 + 1; })");
  assert(1, ({ 2 == 2; }), "({ 2 == 2; })");
  assert(0, ({ 1 == 2; }), "({ 1 == 2; })");
  assert(1, ({ 2 != 1; }), "({ 2 != 1; })");
  assert(0, ({ 2 != 2; }), "({ 2 != 2; })");
  assert(1, ({ 1 < 2; }), "({ 1 < 2; })");
  assert(0, ({ 1 < 1; }), "({ 1 < 1; })");
  assert(1, ({ 2 <= 2; }), "({ 2 <= 2; })");
  assert(0, ({ 2 <= 1; }), "({ 2 <= 1; })");
  assert(0, ({ 1 > 2; }), "({ 1 > 2; })");
  assert(1, ({ 2 > 1; }), "({ 2 > 1; })");
  assert(1, ({ 2 >= 2; }), "({ 2 >= 2; })");
  assert(0, ({ 1 >= 2; }), "({ 1 >= 2; })");
  assert(1, ({ 1 + 2 * 15 / 3 + 2 - 1 == 12 * 100 / 100; }), "({ 1 + 2 * 15 / 3 + 2 - 1 == 12 * 100 / 100; })");
  assert(1, ({ 1 + 2 * 15 / 3 + 2 - 1 > 12 * 100 / 100 - 1; }), "({ 1 + 2 * 15 / 3 + 2 - 1 > 12 * 100 / 100 - 1; })");
  assert(1, ({ int a; int b; a=1; b=2; b-a; }), "({ int a; int b; a=1; b=2; b-a; })");
  assert(50, ({ int a=1000; int b=20; int c; c=a/b; }), "({ int a=1000; int b=20; int c; c=a/b; })");
  assert(100, ({ int a=1000; int b=20; int c=a/b; (c + 50) * (10 - 9); }), "({ int a=1000; int b=20; int c=a/b; (c + 50) * (10 - 9); })");
  assert(5, ({ int num = 1; num * 5; }), "({ int num = 1; num * 5; })");
  assert(5, ({ int num0 = 1; int num1 = num0 > 0; num0 * num1 * 5; }), "({ int num0 = 1; int num1 = num0 > 0; num0 * num1 * 5; })");
  assert(10, ({ int _NUM_TEST; _NUM_TEST = 10; }), "({ int _NUM_TEST; _NUM_TEST = 10; })");
  assert(10, ({ 0; 10; }), "({ 0; 10; })");

  assert(10, fast_return(), "fast_return()");
  assert(5, ({ int num0; int num1; num0 = 1; num1 = num0 > 0; num0 * num1 * 5; }),
         "({ int num0; int num1; num0 = 1; num1 = num0 > 0; num0 * num1 * 5; })");
  assert(5, -(-5), "-(-5)");
  assert(1, ({ int v0=1; int v1=1; int v2=1; int v3=1; int v4=1; int v5=1; int v6=1; int v7=1; int v8=1; int v9=1; int v10=1; v0; }),
         "({ int v0=1; int v1=1; int v2=1; int v3=1; int v4=1; int v5=1; int v6=1; int v7=1; int v8=1; int v9=1; int v10=1; v0; })");
  assert(1, use_if(), "use_if()");
  assert(5, ({ int a; int b; int c; a = 5; b = 1; if (b - a < 0) c=a*b; else c=0; c; }),
         "({ int a; int b; int c; a = 5; b = 1; if (b - a < 0) c=a*b; else c=0; c; })");
  assert(1, ({ int a; a=10; int x; if(a==0)x=0;else if(a==10)x=1; else x=3; x; }),
         "({ int a; a=10; int x; if(a==0)x=0;else if(a==10)x=1; else x=3; x; })");
  assert(3, ({ int a; a=10; int x; if(a==0)x=0;else if(a==1)x=1; else x=3; x; }),
         "({ int a; a=10; int x; if(a==0)x=0;else if(a==1)x=1; else x=3; x; })");
  assert(9, ({ int a; a=10; a = a - 1; a; }), "({ int a; a=10; a = a - 1; a; }),");

  assert(2, ({ int a; a=10; while(a > 2) a = a - 1; a; }), "({ int a; a=10; while(a > 2) a = a - 1; a; })");
  assert(1, ({ int a; a=10; while(a > 3) a = a - 3; a; }), "({ int a; a=10; while(a > 3) a = a - 3; a; })");
  assert(5, ({ int x; int a; a=10; while(a > 0) if (a == 5) {x=a; a=a-1;} else a = a - 1; x; }),
         "({ int x; int a; a=10; while(a > 0) {if (a == 5) {x=a; a=a-1;} else {a = a - 1; x=a;}} x; })");
  assert(10, ({ int a; int sum; a=0; sum=0; for (a=0;a<5;a=a+1) sum = sum+a; sum; }),
         "({ int a; int sum; a=0; sum=0; for (a=0;a<5;a=a+1) sum = sum+a; sum; })");
  assert(10, ({ int a; int sum; sum=0; for (a=0;a<5;a=a+1) sum = sum+a; sum; }),
         "({ int a; int sum; sum=0; for (a=0;a<5;a=a+1) sum = sum+a; sum; })");
  assert(0, ret_0(), "ret_0()");
  assert(10, ({ int a; for (a=0;a<5;) a = a + 10; a; }), "({ int a; for (a=0;a<5;) a = a + 10; a; })");
  assert(10, ({ int a; a=0; for (;a<10;a=a+1) a; a; }), "({ int a; a=0; for (;a<10;a=a+1) a; a; })");
  assert(10, ret_10(), "ret_10()");
  assert(3, ({ int a; int sum; sum=0; for (a=0;a<3;a=a+1){sum = sum + a;} sum; }),
         "({ int a; int sum; sum=0; for (a=0;a<3;a=a+1){sum = sum + a;} sum; })");
  assert(33, ({ int a; int b; int sum; sum=0; a=10; for(b=0;b<3;b=b+1){sum=sum+b; sum=sum+a;} sum; }),
         "({ int a; int b; int sum; sum=0; a=10; for(b=0;b<3;b=b+1){sum=sum+b; sum=sum+a;} sum; })");
  assert(15, ({ int i; int sum; sum=0; i=1; while(i<=5){sum=sum+i; i=i+1;} sum; }),
         "({ int i; int sum; sum=0; i=1; while(i<=5){sum=sum+i; i=i+1;} sum; })");
  assert(0, ({ int i; for(i=0;i<5;i=i+1){} 0; }), "({ int i; for(i=0;i<5;i=i+1){} 0; })");
  assert(2, ({ int a; a=0; if(a==0)a=2; a; }), "({ int a; a=0; if(a==0)a=2; a; })");
  assert(2, ({ int a; a=0; if(a==0){a=2;} a; }), "({ int a; a=0; if(a==0){a=2;} a; })");
  assert(8, ({ int a; a=0; if(a==0){a=2; a=a*2; a=a*2;} a; }), " ({ int a; a=0; if(a==0){a=2; a=a*2; a=a*2;} a; })");

  assert(3, ret3(), "ret3()");
  assert(5, ({ ret5(); }), "({ ret5(); })");
  assert(1, return_1(), "return_1()");
  assert(3, add(1, 2), "add(1, 2)");
  assert(6, sub(10, 4), "sub(10, 4)");
  assert(10, add5(2, 1, 1, 1, 5), "add5(2, 1, 1, 1, 5)");
  assert(5, retx(5), "retx(5)");
  assert(5, ({ int x=5; retx(x); }), "({ int x=5; retx(x); })");
  assert(55, fib(9), "fib(9)");

  assert(3, ({int x=3; int *y=&x; *y; }), "({int x=3; int *y=&x; *y; })");
  assert(5, ({int x=5; int *y=&x; int **z=&y; **z; }), "({int x=5; int *y=&x; int **z=&y; **z; })");
  assert(10, ({int x=10; int y=5; int *z=&y-1; *z; }), "({int x=10; int y=5; int *z=&y-1; *z; })");
  assert(5, ({int x=5; *&x; }), "({int x=5; *&x; })");
  assert(10, ({ int x = 5; int *px = &x; *px = 10; *px; }), "({ int x = 5; int *px = &x; *px = 10; *px; })");
  assert(10, ({ int x = 5; int *px = &x; *px = 10; x; }), "({ int x = 5; int *px = &x; *px = 10; x; })");
  assert(7, ({ int x=3; int y=5; *(&x+1)=7; y; }), "({ int x=3; int y=5; *(&x+1)=7; y; })");
  assert(7, ({ int x=3; int y=5; *(&y-1)=7; x; }), "({ int x=3; int y=5; *(&y-1)=7; x; })");
  assert(5, ({ int x = 3; (&x+5) - (&x); }), "({ int x = 3; (&x+5) - (&x); })");

  assert(4, ({ int x = 0; sizeof(x); }), "({ int x = 0; sizeof(x); })");
  assert(4, ({ sizeof(1); }), "({ sizeof(1); })");
  assert(8, ({ sizeof(sizeof(1)); }), "({ sizeof(sizeof(1)); })");
  assert(8, ({ int x = 0; sizeof(&x); }), "({ int x = 0; sizeof(&x); })");
  assert(8, ({ int x = 0; sizeof(&x + 2); }), "({ int x = 0; sizeof(&x + 2); })");
  assert(8, ({ int x = 0; sizeof(sizeof(&x)); }), "({ int x = 0; sizeof(sizeof(&x)); })");
  assert(11, ({ int a[3]; *a=10; *(a+1)=1; *(a+2)=*a+*(a+1); *(a+2); }), "({ int a[3]; *a=10; *(a+1)=1; *(a+2)=*a+*(a+1); *(a+2); })");
  assert(11, ({ int a[3]; int *b=a+2; *b=11; *(a+2); }), "({ int a[3]; int *b=a+2; *b=11; *(a+2); })");
  assert(32, ({ int a[8]; sizeof(a); }), "({ int a[8]; sizeof(a); })");
  assert(8, ({ int a[8]; int *b = a; sizeof(b); }), "({ int a[8]; int *b = a; sizeof(b); })");
  assert(0, ({ int x[2][3]; int *y=x; *y=0; **x; }), "({ int x[2][3]; int *y=x; *y=0; **x; })");
  assert(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }), "({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); })");
  assert(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }), "({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); })");
  assert(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }), "({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); })");
  assert(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }), "({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); })");
  assert(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }), "({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); })");
  assert(11, ({ int a[100]; a[99]=11; a[99]; }), "({ int a[100]; a[99]=11; a[99]; })");
  assert(11, ({ int a[3]; a[0]=10; a[1]=1; a[2]=*a+*(a+1); *(a+2); }), "({ int a[3]; a[0]=10; a[1]=1; a[2]=*a+*(a+1); *(a+2); })");
  assert(11, ({ int a[3]; int *b=a+2; b[0]=11; a[2]; }), "({ int a[3]; int *b=a+2; b[0]=11; a[2]; })");
  assert(0, ({ int x[2][3]; x[0][0]=0; **x; }), "({ int x[2][3]; x[0][0]=0; **x; })");
  assert(1, ({ int x[2][3]; x[0][1]=1; *(*x+1); }), "({ int x[2][3]; x[0][1]=1; *(*x+1); })");
  assert(2, ({ int x[2][3]; x[0][2]=2; *(*x+2); }), "({ int x[2][3]; x[0][2]=2; *(*x+2); })");
  assert(3, ({ int x[2][3]; x[1][0]=3; **(x+1); }), "({ int x[2][3]; x[1][0]=3; **(x+1); })");
  assert(4, ({ int x[2][3]; x[1][1]=4; *(*(x+1)+1); }), "({ int x[2][3]; x[1][1]=4; *(*(x+1)+1); })");
  assert(5, ({ int x[2][3]; x[1][2]=5; *(*(x+1)+2); }), "({ int x[2][3]; x[1][2]=5; *(*(x+1)+2); })");
  assert(1, ({ int x[2][3]; x[0][1]=1; x[0][1]; }), "({ int x[2][3]; x[0][1]=1; x[0][1]; })");
  assert(2, ({ int x[2][3]; x[0][2]=2; x[0][2]; }), "({ int x[2][3]; x[0][2]=2; x[0][2]; })");
  assert(3, ({ int x[2][3]; x[1][0]=3; x[1][0]; }), "({ int x[2][3]; x[1][0]=3; x[1][0]; })");
  assert(4, ({ int x[2][3]; x[1][1]=4; x[1][1]; }), "({ int x[2][3]; x[1][1]=4; x[1][1]; })");
  assert(5, ({ int x[2][3]; x[1][2]=5; x[1][2]; }), "({ int x[2][3]; x[1][2]=5; x[1][2]; })");
  assert(1, ({ int x[2][3]; x[0][1]=1; *(*x+1); }), "({ int x[2][3]; x[0][1]=1; *(*x+1); })");
  assert(2, ({ int x[2][3]; x[0][2]=2; *(*x+2); }), "({ int x[2][3]; x[0][2]=2; *(*x+2); })");
  assert(3, ({ int x[2][3]; x[0][3]=3; **(x+1); }), "({ int x[2][3]; x[0][3]=3; **(x+1); })");
  assert(4, ({ int x[2][3]; x[0][4]=4; *(*(x+1)+1); }), "({ int x[2][3]; x[0][4]=4; *(*(x+1)+1); })");
  assert(5, ({ int x[2][3]; x[0][5]=5; *(*(x+1)+2); }), "({ int x[2][3]; x[0][5]=5; *(*(x+1)+2); })");
  assert(5, ({int x[3]; x[0]=1;x[1]=2;x[2]=2; sum_arr_elems(x); }), "{int x[3]; x[0]=1;x[1]=2;x[2]=2; sum_arr_elems(x);}");

  assert(0, ({ g1=0; g1; }), "({ g1=0; g1; })");
  assert(1, ({ g1 = 1; g1; }), "({ g1 = 1; g1; })");
  assert(10, ({ g1 = 10; }), "({ g1 = 10;})");
  assert(4, sizeof(g1), "sizeof(g1)");
  assert(0, ({g2[0]=0; g2[0]; }), "({g2[0]=0; g2[0]; })");
  assert(1, ({g2[1]=1; g2[1]; }), "({g2[1]=1; g2[1]; })");
  assert(2, ({g2[2]=2; g2[2]; }), "({g2[2]=2; g2[2]; })");

  assert(1, ({ char x=1; x; }), "({ char x=1; x; })");
  assert(1, ({ char x=1; char y=2; x; }), "({ char x=1; char y=2; x; })");
  assert(2, ({ char x=1; char y=2; y; }), "({ char x=1; char y=2; y; })");
  assert(1, ({ char x=1; sizeof(x); }), "({ char x=1; sizeof(x); })");
  assert(10, ({ char x[10]; sizeof(x); }), "({ char x[10]; sizeof(x); })");
  assert(5, ({ char x[10]; char *y=x+3; *y=5; x[3]; }), "({ char x[10]; char *y=x+3; *y=5; x[3]; })");
  assert(5, ({ char a=1; char b=4; char c=a+b; c; }), "({ char a=1; char b=4; char c=a+b; c; })");
  assert(5, ({ char a=1; char b=4; int c=a+b; c; }), "({ char a=1; char b=4; int c=a+b; c; })");
  assert(5, ({ int a=1; int b=4; char c=a+b; c; }), "({ int a=1; int b=4; char c=a+b; c; })");
  assert(5, ({ int a=1; char b=4; char c=a+b; c; }), "({ int a=1; char b=4; char c=a+b; c; })");
  assert(15, add_char1(7, 5, 3), "add_char1(7, 5, 3)");
  assert(15, add_char2(7, 5, 3), "add_char2(7, 5, 3)");
  assert(10, ({ g3 = 10; g3; }), "({ g3 = 10; g3; })");
  assert(1, ({g4[1]=1; g4[1]; }), "({g4[1]=1; g4[1]; })");

  assert(97, ({ "abc"[0]; }), "({ \"abc\"[0]; })");
  assert(98, ({ "abc"[1]; }), "({ \"abc\"[1]; })");
  assert(99, ({ "abc"[2]; }), "({ \"abc\"[2]; })");
  assert(0, ({ "abc"[3]; }), "({ \"abc\"[3]; })");
  assert(97, ({ char *str = "abc"; str[0]; }), "({ char *str = \"abc\"; str[0]; })");
  assert(98, ({ char *str = "abc"; str[1]; }), "({ char *str = \"abc\"; str[1]; })");
  assert(99, ({ char *str = "abc"; str[2]; }), "({ char *str = \"abc\"; str[2]; })");
  assert(0, ({ char *str = "abc"; str[3]; }), "({ char *str = \"abc\"; str[3]; })");
  assert(4, ({ sizeof("abc"); }), "({ sizeof(\"abc ");
  assert(7, "\a"[0], "\"\\a\"[0]");
  assert(8, "\b"[0], "\"\\b\"[0]");
  assert(9, "\t"[0], "\"\\t\"[0]");
  assert(10, "\n"[0], "\"\\n\"[0]");
  assert(11, "\v"[0], "\"\\v\"[0]");
  assert(12, "\f"[0], "\"\\f\"[0]");
  assert(13, "\r"[0], "\"\\r\"[0]");
  assert(27, "\e"[0], "\"\\e\"[0]");
  assert(7, "\ax\ny"[0], "\"\\ax\\ny\"[0]");
  assert(120, "\ax\ny"[1], "\"\\ax\\ny\"[1]");
  assert(10, "\ax\ny"[2], "\\ax\\ny\"[2]");
  assert(121, "\ax\ny"[3], "\"\\ax\\ny\"[3]");
  assert(0, "\0"[0], "\"\\0\"[0]");
  assert(16, "\20"[0], "\"\\20\"[0]");
  assert(65, "\101"[0], "\"\\101\"[0]");
  assert(104, "\1500"[0], "\"\\1500\"[0]");
  assert(10, "\xA"[0], "\"\\xA\"[0]");
  assert(0, "\x00"[0], "\"\\x00\"[0]");
  assert(119, "\x77"[0], "\"\\x77\"[0]");

  assert(10, ({ int x=1, y=9; x+y; }), "({ int x=1, y=9; x+y; })");
  assert(10, ({ int x, y; x=1; y=9; x+y; }), "({ int x, y; x=1; y=9; x+y; })");
  assert(10, ({ int x=1, y; x; y=9; x+y; }), "({ int x=1, y; x; y=9; x+y; })");
  assert(10, ({ int x, y=9; x=1; x+y; }), "({ int x, y=9; x=1; x+y; })");
  assert(10, ({int x[2], y=8; x[0]=1; x[1]=1; x[0]+x[1]+y; }), "({int x[2], y=8; x[0]=1; x[1]=1; x[0]+x[1]+y;})");
  assert(10, ({ char x=1, y=9; x+y; }), "({ char x=1, y=9; x+y; })");
  assert(10, ({ char x, y; x=1; y=9; x+y; }), "({ char x, y; x=1; y=9; x+y; })");
  assert(10, ({ char x=1, y; x; y=9; x+y; }), "({ char x=1, y; x; y=9; x+y; })");
  assert(10, ({ char x, y=9; x=1; x+y; }), "({ char x, y=9; x=1; x+y; })");
  assert(10, ({char x[2], y=8; x[0]=1; x[1]=1; x[0]+x[1]+y; }), "({char x[2], y=8; x[0]=1; x[1]=1; x[0]+x[1]+y;})");
  assert(10, ({ int x = 1, y = 8, z = 1; x + y + z; }), "({ int x = 1, y = 8, z = 1; x + y + z; })");

  assert(3, (1, 2, 3), "(1, 2, 3)");
  assert(10, ({ int x, y=(x=1,9); x+y; }), "({ int x, y=(x=1,9); x+y; })");
  assert(10, ({ int x=0, y; (x=1, y)=9; x+y; }), "({ int x=0, y; (x=1, y)=9; x+y; })");

  assert(21, add6(1, 2, 3, 4, 5, 6), "add6(1, 2, 3, 4, 5, 6)");

  assert(1, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; }), "({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; })");
  assert(2, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; }), "({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; })");
  assert(1, ({ struct {int a; int b;} x[2]; x[1].a=1; x[1].b=2; x[1].a; }), "({ struct {int a; int b;} x[2]; x[1].a=1; x[1].b=2; x[1].a; })");
  assert(2, ({ struct {int a; int b;} x[2]; x[1].a=1; x[1].b=2; x[1].b; }), "({ struct {int a; int b;} x[2]; x[1].a=1; x[1].b=2; x[1].b; })");
  assert(1, ({ struct {int a; char b;} x; x.a=1; x.b=2; x.a; }), "({ struct {int a; char b;} x; x.a=1; x.b=2; x.a; })");
  assert(2, ({ struct {int a; char b;} x; x.a=1; x.b=2; x.b; }), "({ struct {int a; char b;} x; x.a=1; x.b=2; x.b; })");
  assert(1, ({ struct {int a; struct {char b; char c;} y;} x; x.a=1; x.y.b=2; x.y.c=3; x.a; }), "({ struct {int a; struct {char b; char c;} y;} x; x.a=1; x.y.b=2; x.y.c=3; x.a; })");
  assert(2, ({ struct {int a; struct {char b; char c;} y;} x; x.a=1; x.y.b=2; x.y.c=3; x.y.b; }), "({ struct {int a; struct {char b; char c;} y;} x; x.a=1; x.y.b=2; x.y.c=3; x.y.b; })");
  assert(3, ({ struct {int a; struct {char b; char c;} y;} x; x.a=1; x.y.b=2; x.y.c=3; x.y.c; }), "({ struct {int a; struct {char b; char c;} y;} x; x.a=1; x.y.b=2; x.y.c=3; x.y.c; })");
  assert(3, ({ struct {int a; struct {char b; char c;} y;} x; x.y.c=3; char *p=&x; *(p+5); }), "({ struct {int a; struct {char b; char c;} y;} x; x.y.c=3; char *p=&x; *(p+9); })");
  assert(3, ({ struct {int a; struct {char b; char c[5];} y[2];} x; x.y[1].c[3]=3; x.y[1].c[3]; }), "({ struct {int a; struct {char b; char c[5];} y[2];} x; x.y[1].c[3]=3; x.y[1].c[3]; })");
  assert(1, ({ struct {int a, b;} x; x.a=1; x.b=2; x.a; }), "({ struct {int a, b;} x; x.a=1; x.b=2; x.a; })");
  assert(2, ({ struct {int a, b;} x; x.a=1; x.b=2; x.b; }), "({ struct {int a, b;} x; x.a=1; x.b=2; x.b; })");
  assert(1, ({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.a; }), "({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.a; })");
  assert(2, ({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.b; }), "({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.b; })");
  assert(3, ({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.c; }), "({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.c; })");
  assert(4, ({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.d; }), "({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.d; })");
  assert(5, ({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.e; }), "({ struct {int a, b; char c, d, e, f;} x; x.a=1; x.b=2; x.c=3; x.d=4; x.e=5; x.e; })");
  assert(3, ({ struct {char a; int b;} x; x.b = 3; char *p=&x.a; *(p+4); }), "({ struct {char a; int b;} x; x.b = 3; char *p=&x.a; *(p+8); })");
  assert(8, ({ struct {int a; int b;} x; sizeof(x); }), "({ struct {int a; int b;} x; sizeof(x); })");
  assert(8, ({ struct {int a; char b;} x; sizeof(x); }), "({ struct {int a; char b;} x; sizeof(x); })");
  assert(16, ({ struct {int a; int b;} x[2]; sizeof(x); }), "({ struct {int a; int b;} x[2]; sizeof(x); })");
  assert(16, ({ struct {int a; char b;} x[2]; sizeof(x); }), "({ struct {int a; char b;} x[2]; sizeof(x); })");
  assert(8, ({ struct {int a; struct {char b; char c;} y;} x; sizeof(x); }), "({ struct {int a; struct {char b; char c;} y;} sizeof(x); })");
  assert(16, ({ struct {int a; struct {char b; char c[5];} y[2];} x; sizeof(x); }), "({ struct {int a; struct {char b; char c[5];} y[2];} x; sizeof(x); })");
  assert(12, ({ struct {int a, b; char c, d, e, f;} x; sizeof(x); }), "({ struct {int a, b; char c, d, e, f;} x; sizeof(x); })");
  assert(8, ({ struct {char a; int b;} x; sizeof(x); }), "({ struct {char a; int b;} x; sizeof(x); })");

  assert(8, ({ int x; int y; int z; char *a=&x; char *b=&y; char *c=&z; c-a; }), "({ int x; int y; int z; char *a=&x; char *b=&y; char *c=&z; c-a; })");
  assert(8, ({ int x; char y; int z; char *a=&x; char *b=&y; char *c=&z; c-a; }), "({ int x; char y; int z; char *a=&x; char *b=&y; char *c=&z; c-a; })");
  assert(7, ({ int x; int y; char z; char *a=&y; char *b=&z; b-a; }), "({ int x; int y; char z; char *a=&y; char *b=&z; b-a; })");
  assert(1, ({ int x; char y; int z; char *a=&y; char *b=&z; b-a; }), "({ int x; char y; int z; char *a=&y; char *b=&z; b-a; })");

  assert(8, ({ struct foo {int x, y;} x; struct foo y; sizeof(y); }), "({ struct foo {int x, y;} x; struct foo y; sizeof(y); })");
  assert(5, ({ struct foo {int x, y;} x; struct foo y; y.y=5; y.y; }), "({ struct foo {int x, y;} x; struct foo y; y.y=5; y.y; })");
  assert(5, ({ int; int x=5; x; }), "({ int; int x=5; x; })");
  assert(8, ({ struct foo {int x, y;}; struct foo y; sizeof(y); }), "({ struct foo {int x, y;}; struct foo y; sizeof(y); })");

  assert(5, ({ struct foo {int x, y;}; struct foo y; y.y=5; struct foo *z=&y; z->y; }), "({ struct foo {int x, y;}; struct foo y; y.y=5; struct foo *z=&y; z->y; })");
  assert(5, ({ struct foo {int x, y;}; struct foo y; struct foo *z=&y; z->y=5; y.y=5; }), "({ struct foo {int x, y;}; struct foo y; struct foo *z=&y; z->y=5; y.y=5; })");

  assert(8, ({union {int a; char b[6];}x; sizeof(x); }), "({union {int a; char b[6];}x; sizeof(x); })");
  assert(3, ({union {int a; char b[4];}x; x.a = 515; x.b[0]; }), "({union {int a; char b[4];}x; x.a = 515; x.b[0]; })");
  assert(2, ({union {int a; char b[4];}x; x.a = 515; x.b[1]; }), "({union {int a; char b[4];}x; x.a = 515; x.b[1]; })");
  assert(0, ({union {int a; char b[4];}x; x.a = 515; x.b[2]; }), "({union {int a; char b[4];}x; x.a = 515; x.b[2]; })");
  assert(0, ({union {int a; char b[4];}x; x.a = 515; x.b[3]; }), "({union {int a; char b[4];}x; x.a = 515; x.b[3]; })");

  assert(1, ({ struct t {int a; int b;} x; x.a=1; x.b=2; struct t y=x; x.a; }), "({ struct t {int a; int b;} x; x.a=1; x.b=2; struct t y=x; x.a; })");
  assert(2, ({ struct t {int a; int b;} x; x.a=1; x.b=2; struct t y=x; x.b; }), "({ struct t {int a; int b;} x; x.a=1; x.b=2; struct t y=x; x.b; })");
  assert(3, ({ struct t {int a; int b;} x; x.a=3; struct t y; struct t *z=&y; *z=x; y.a; }), "({ struct t {int a; int b;} x; x.a=3; struct t y; struct t *z=&y; *z=x; y.a; })");

  assert(2, ({short x; sizeof(x); }), "({short x; sizeof(x); })");
  assert(4, ({struct {char a; short b; } x; sizeof(x); }), "({struct {char a; short b; } x; sizeof(x); })");
  assert(8, ({long x; sizeof(x); }), "({long x; sizeof(x); })");
  assert(16, ({struct {char a; long b; } x; sizeof(x); }), "({struct {char a; long b; } x; sizeof(x); })");
  assert(1, sub_short(3, 1, 1), "sub_short(3, 1, 1)");
  assert(1, sub_long(3, 1, 1), "sub_long(3, 1, 1)");

  assert(24, ({int *x[3]; sizeof(x); }), "({int *x[3]; sizeof(x); })");
  assert(8, ({int (*x)[3]; sizeof(x); }), "({int (*x)[3]; sizeof(x); })");
  assert(3, ({int *x[3]; int y; x[0]=&y; y=3; x[0][0]; }), "({int *x[3]; int y; x[0]=&y; y=3; x[0][0]; })");
  assert(4, ({int *x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; }), "({int *x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; })");
  assert(777, ret_777(), "ret_777()");

  {
    void *x;
  }

  assert(1, ({char x; sizeof(x); }), "({char x; sizeof(x); })");
  assert(2, ({short int x; sizeof(x); }), "({short int x; sizeof(x); })");
  assert(2, ({int short x; sizeof(x); }), "({int short x; sizeof(x); })");
  assert(4, ({int x; sizeof(x); }), "({int x; sizeof(x); })");
  assert(8, ({long int x; sizeof(x); }), "({long int x; sizeof(x); })");
  assert(8, ({int long x; sizeof(x); }), "({int long x; sizeof(x); })");
  assert(8, ({long long x; sizeof(x); }), "({long long x; sizeof(x); })");
  assert(8, ({long long int x; sizeof(x); }), "({long long int x; sizeof(x); })");
  assert(8, ({long int long x; sizeof(x); }), "({long int long x; sizeof(x); })");

  assert(1, ({ typedef int t; t x=1; x; }), "({typedef int t; t x=1; x; })");
  assert(1, ({ typedef struct {int a;} t; t x; x.a=1; x.a; }), "({typedef struct {int a;} t; t x; x.a=1; x.a; })");
  assert(1, ({ typedef int t; t t = 1; t; }), "({ typedef int t; t t = 1; t; })");
  assert(2, ({ typedef struct {int a;} t; {typedef int t;} t x; x.a=2; x.a; }), "({ typedef struct {int a;} t; {typedef int t;} t x; x.a=2; x.a; })");
  assert(4, ({ typedef t; t x; sizeof(x); }), "({ typedef t; t x; sizeof(x); })");
  assert(4, ({ typedef typedef t; t x; sizeof(x); }), "({ typedef typedef t; t x; sizeof(x); })");
  assert(3, ({ MyInt x=3; x; }), "({ MyInt x=3; x; })");
  assert(16, ({ MyInt2 x; sizeof(x); }), "({ MyInt2 x; sizeof(x); })");

  assert(1, ({ sizeof(char); }), "({ sizeof(char); })");
  assert(2, ({ sizeof(short); }), "({ sizeof(short); })");
  assert(2, ({ sizeof(short int); }), "({ sizeof(short int); })");
  assert(2, ({ sizeof(int short); }), "({ sizeof(int short); })");
  assert(4, ({ sizeof(int); }), "({ sizeof(int); })");
  assert(8, ({ sizeof(long); }), "({ sizeof(long); })");
  assert(8, ({ sizeof(long int); }), "({ sizeof(long int); })");
  assert(8, ({ sizeof(int long); }), "({ sizeof(int long); })");
  assert(8, ({ sizeof(char *); }), "({ sizeof(char *); })");
  assert(8, ({ sizeof(int *); }), "({ sizeof(int *); })");
  assert(8, ({ sizeof(long *); }), "({ sizeof(long *); })");
  assert(8, ({ sizeof(int **); }), "({ sizeof(int **); })");
  assert(8, ({ sizeof(int(*)[4]); }), "({ sizeof(int(*)[4]); })");
  assert(16, ({ sizeof(int[4]); }), "({ sizeof(int[4]); })");
  assert(24, ({ sizeof(int[2][3]); }), "({ sizeof(int[2][3]); })");
  assert(8, ({ sizeof(struct { int a; int b; }); }), "({ sizeof(struct { int a; int b; }); })");

  assert(131586, (int)8590066178, "(int)8590066178");
  assert(514, (short)8590066178, "(short)8590066178");
  assert(2, (char)8590066178, "(char)8590066178");
  assert(1, (char)1, "(char)1");
  assert(1, (short)1, "(short)1");
  assert(1, (int)1, "(int)1");
  assert(1, (long)((int)1), "(long)((int)1)");
  assert(1, (long)&*(int *)1, "(long)&*(int *)1");
  assert(1, ({ int x = 8193; *(char *)(&x); }), "({ int x = 8193; *(char *)(&x); })");
  assert(32, ({ int x = 8193; *((char *)(&x) + 1); }), "({ int x = 8193; *((char *)(&x) + 1); })");
  assert(0, ({ int x = 8193; *((char *)(&x) + 2); }), "({ int x = 8193; *((char *)(&x) + 2); })");
  assert(0, ({ int x = 8193; *((char *)(&x) + 3); }), "({ int x = 8193; *((char *)(&x) + 3); })");

  assert(4, sizeof(-10 + 5), "sizeof(-10 + 5)");
  assert(4, sizeof(-10 - 5), "sizeof(-10 + 5)");
  assert(4, sizeof(-10 * 5), "sizeof(-10 + 5)");
  assert(4, sizeof(-10 / 5), "sizeof(-10 + 5)");

  assert(8, sizeof(-10 + (long)5), "sizeof(-10 + 5)");
  assert(8, sizeof(-10 - (long)5), "sizeof(-10 + 5)");
  assert(8, sizeof(-10 * (long)5), "sizeof(-10 + 5)");
  assert(8, sizeof(-10 / (long)5), "sizeof(-10 + 5)");
  assert(8, sizeof((long)-10 + 5), "sizeof(-10 + 5)");
  assert(8, sizeof((long)-10 - 5), "sizeof(-10 + 5)");
  assert(8, sizeof((long)-10 * 5), "sizeof(-10 + 5)");
  assert(8, sizeof((long)-10 / 5), "sizeof(-10 + 5)");

  assert(3, ({ g1 = 3; *g1_ptr(); }), "({ g1 = 3; *g1_ptr(); })");
  assert(261, int_to_int(261), "int_to_int(261)");
  assert(261, ({ int x = int_to_int(261); x; }), "({ int x = int_to_int(261); x; })");
  assert(5, int_to_char(261), "int_to_char(261)");
  assert(5, ({ int x = int_to_char(261); x; }), "({ int x = int_to_char(261); x; })");

  assert(-5, div_long(-10, 2), "div_long(-10, 2)");

  assert(0, ({ _Bool x=0; x; }), "({ _Bool x=0; x; })");
  assert(1, ({ _Bool x=1; x; }), "({ _Bool x=1; x; })");
  assert(1, ({ _Bool x=5; x; }), "({ _Bool x=5; x; })");
  assert(1, (_Bool)1, "(_Bool)1");
  assert(1, (_Bool)5, "(_Bool)5");
  assert(0, (_Bool)(char)256, "(_Bool)(char)256");
  assert(1, bool_fn_add(3), "bool_fn_add(3)");
  assert(0, bool_fn_sub(3), "bool_fn_add(3)");
  assert(1, bool_fn_add(-3), "bool_fn_add(-3)");
  assert(0, bool_fn_sub(-3), "bool_fn_add(-3)");
  assert(1, bool_fn_add(0), "bool_fn_add(0)");
  assert(1, bool_fn_sub(0), "bool_fn_add(0)");

  assert(97, 'a', "'a'");
  assert(10, '\n', "'\\n'");
  assert(4, sizeof('a'), "sizeof('a')");

  assert(0, ({ enum {zero, one, two}; zero; }), "({ enum {zero, one, two}; zero; })");
  assert(1, ({ enum {zero, one, two}; one; }), "({ enum {zero, one, two}; one; })");
  assert(2, ({ enum {zero, one, two}; two; }), "({ enum {zero, one, two}; two; })");
  assert(5, ({ enum {five = 5, six, seven}; five; }), "({ enum {five = 5, six, seven}; five; })");
  assert(6, ({ enum {five = 5, six, seven}; six; }), "({ enum {five = 5, six, seven}; six; })");
  assert(7, ({ enum {five = 5, six, seven}; seven; }), "({ enum {five = 5, six, seven}; seven; })");
  assert(0, ({ enum {zero, five=5, six}; zero; }), "({ enum {zero, five=5, six}; zero; })");
  assert(5, ({ enum {zero, five=5, six}; five; }), "({ enum {zero, five=5, six}; five; })");
  assert(6, ({ enum {zero, five=5, six}; six; }), "({ enum {zero, five=5, six}; six; })");
  assert(3, ({ enum {zero, five=5, three=3, four}; three; }), "({ enum {zero, five=5, three=3, four}; three; })");
  assert(4, ({ enum {zero, five=5, three=3, four}; four; }), "({ enum {zero, five=5, three=3, four}; four; })");
  assert(4, ({ enum {zero, one, two} x; sizeof(x); }), "({ enum {zero, one, two} x; sizeof(x); })");
  assert(4, ({ enum t {zero, one, two}; enum t x; sizeof(x); }), "({ enum t {zero, one, two}; t x; sizeof(x); })");

  assert(3, static_fn(), "static_fn()");

  assert(45, ({ int j=0; for (int i=0; i<10; i=i+1) j=j+i; j; }), "({ int j=0; for (int i=0;i<10;i=i+1;)j=j+i; j; })");
  assert(6, ({ int i=6, j=0; for (int i=0; i<10; i=i+1) j=j+i; i; }), "({ int i=3, j=0; for (int i=0; i<10; i=i+1) j=j+i; i; })");

  assert(8, ({ int i=6; i+=2; i; }), "({ int i=6; i+=2; i; })");
  assert(4, ({ int i=6; i-=2; i; }), "({ int i=6; i-=2; i; })");
  assert(12, ({ int i=6; i*=2; i; }), "({ int i=6; i*=2; i; })");
  assert(3, ({ int i=6; i/=2; i; }), "({ int i=6; i/=2; i; })");
  assert(8, ({ int i=6; i+=2; }), "({ int i=6; i+=2; })");
  assert(4, ({ int i=6; i-=2; }), "({ int i=6; i-=2; })");
  assert(12, ({ int i=6; i*=2; }), "({ int i=6; i*=2; })");
  assert(3, ({ int i=6; i/=2; }), "({ int i=6; i/=2; })");
  assert(2, ({ int i=1; ++i; }), "({ int i=1; ++i; })");
  assert(0, ({ int i=1; --i; }), "({ int i=1; --i; })");
  assert(3, ({ int i=1; 1+(++i); }), "({ int i=1; 1+(++i); })");
  assert(21, ({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; ++*p; }), "({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; ++*p; })");
  assert(19, ({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; --*p; }), "({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; --*p; })");
  assert(1, ({ int i=1; i++; }), "({ int i=1; i++; })");
  assert(1, ({ int i=1; i--; }), "({ int i=1; i--; })");
  assert(2, ({ int i=1; 1+(i++); }), "({ int i=1; 1+(i++); })");
  assert(20, ({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; *p++; }), "({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; ++*p; })");
  assert(20, ({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; *p--; }), "({ int a[3]; a[0]=10; a[1]=20; a[2]=30; int *p=a+1; --*p; })");

  assert(4660, 0x1234, "0x1234");
  assert(4820, 0x12D4, "0x12D4");
  assert(4660, 0X1234, "0x1234");
  assert(4820, 0X12D4, "0x12d4");
  assert(13, 0b1101, "0b1101");
  assert(13, 0B1101, "0b1101");
  assert(668, 01234, "01234");

  assert(0, !1, "!1");
  assert(0, !2, "!2");
  assert(1, !0, "!0");
  assert(1, !(char)0, "!(char)0");
  assert(0, !(long)5, "!(long)5");
  assert(4, sizeof(!(char)0), "sizeof(!(char)0)");
  assert(4, sizeof(!(long)5), "sizeof(!(long)5)");

  assert(-1, ~0, "~0");
  assert(0, ~-1, "~-1");

  assert(5, 17 % 6, "17%6");
  assert(5, (short)17 % 6, "(short)17%6");
  assert(5, (long)17 % 6, "(long)17%6");
  assert(2, ({ int i=10; i%=4; i; }), "({ int i=10; i%=4; i; })");
  assert(2, ({ short i=10; i%=4; i; }), "({ short i=10; i%=4; i; })");
  assert(2, ({ long i=10; i%=4; i; }), "({ long i=10; i%=4; i; })");

  assert(0, 0 & 1, "0 & 1");
  assert(1, 3 & 1, "3 & 1");
  assert(3, 7 & 3, "7 & 3");
  assert(10, -1 & 10, "-1 & 10");

  assert(1, 0 | 1, "0 | 1");
  assert(0b101010, 0b101000 | 0b000010, "0b101000 | 0b000010");

  assert(1, 0 ^ 1, "0 ^ 1");
  assert(0, 1 ^ 1, "1 ^ 1");
  assert(0b100010, 0b101000 ^ 0b001010, "0b101000 ^ 0b000010");

  assert(1, ({ int i = 5; i &= 3; }), "({ int i = 5; i &= 3; })");
  assert(7, ({ int i = 5; i |= 3; }), "({ int i = 5; i |= 3; })");
  assert(6, ({ int i = 5; i ^= 3; }), "({ int i = 5; i ^= 3; })");

  assert(0, 0 && 1, "0 && 1");
  assert(1, 3 && 1, "3 && 1");
  assert(1, 7 && 3, "7 && 3");
  assert(0, (1 - 1) && 1, "(1 - 1) && 1");
  assert(1, 0 || 1, "0 || 1");
  assert(0, 0 || 0, "0 || 0");
  assert(1, 2 || 1, "2 || 1");
  assert(1, (1 - 1) || 1, "(1 - 1) || 1");

  assert(8, sizeof(int(*)[10]), "sizeof(int(*)[10])");
  assert(8, sizeof(int(*)[][10]), "sizeof(int(*)[][10])");

  assert(2, ({ int x[2]; x[0]=2; param_decay(x); }), "({ int x[2]; x[0]=2; param_decay(x); })");

  assert(8, ({ struct foo *bar; sizeof(bar); }), "({ struct foo *bar; sizeof(bar); })");
  assert(4, ({ struct T *foo; struct T {int x; }; sizeof(struct T); }), "({ struct T *foo; struct T {int x; }; sizeof(struct T); })");
  assert(12, ({ typedef struct T T; struct T { int x, y, z; }; sizeof(T); }), "({ typedef struct T T; struct T { int x, y, z; }; sizeof(T); })");
  assert(14, ({ struct T { struct T *next; int x; } a; struct T b; b.x=14; a.next=&b; a.next->x; }), "({ struct T { struct T *next; int x; } a; struct T b; b.x=14; a.next=&b; a.next->x; })");

  assert(15, ({ int j=0; for (int i=0;i<100;i++) { j+=i; if (i==5) break; } j; }), "({ int j=0; for (int i=0;i<100;i++) { j+=i; if (i==5) break; } j; })");
  assert(9, ({ int sum=0; for (int i=1; i<=2; i++) for (int j=1; j<=2; j++) sum+=i*j; sum; }), "({ int sum=0; for (int i=1; i<=2; i++) for (int j=1; j<=2; j++) sum+=i*j; sum; })");
  assert(15, ({ int i=0, j=0; while(1) {j+=++i; if (i==5) break;} j; }), "({ int i=0, j=0; while(1) {j+=++i; if (i==5) break;} j; })");
  assert(1, ({ int i=1; for(;;)break; i; }), "({ int i=1; for(;;)break; i; })");
  assert(11, ({ int j=0; for (int i=0;i<=5;i++) { if (i==4) continue; j+=i; } j; }), "({ int j=0; for (int i=0;i<=5;i++) { if (i==4) continue; j+=i; } j; })");
  assert(7, ({ int sum=0; for (int i=1; i<=2; i++) for (int j=1; j<=2; j++) { if(i==1 && j==2)continue; sum+=i*j; } sum; }),
         "({ int sum=0; for (int i=1; i<=2; i++) for (int j=1; j<=2; j++) { if(i==1 && j==2)continue; sum+=i*j; } sum; })");
  assert(11, ({ int i=0, j=0; while(1) {i++; if(i==4)continue; j+=i; if (i==5) break;} j; }),
         "({ int i=0, j=0; while(1) {i++; if(i==4)continue; j+=i; if (i==5) break;} j; })");

  assert(3, ({ int i=0; goto a; a: i++; b: i++; c: i++; i; }), "({ int i=0; goto a; a: i++; b: i++; c: i++; i; })");
  assert(2, ({ int i=0; goto e; d: i++; e: i++; f: i++; i; }), "({ int i=0; goto e; d: i++; e: i++; f: i++; i; })");
  assert(1, ({ int i=0; goto i; g: i++; h: i++; i: i++; i; }), "({ int i=0; goto i; g: i++; h: i++; i: i++; i; })");

  assert(5, ({ int i=0; switch(0) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(0) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(6, ({ int i=0; switch(1) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(1) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(7, ({ int i=0; switch(2) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(2) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(0, ({ int i=0; switch(3) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(3) {case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(5, ({ int i=0; switch(0) {case 0:i=5;break; default:i=7; } i; }), "({ int i=0; switch(0) {case 0:i=5; default:i=7; } i; })");
  assert(7, ({ int i=0; switch(1) {case 0:i=5;break; default:i=7; } i; }), "({ int i=0; switch(1) {case 0:i=5; default:i=7; } i; })");
  assert(2, ({ int i=0; switch(1) {case 0: 0; case 1: 0; case 2: 0; i=2; } i; }), "({ int i=0; switch(1) {case 0: 0; case 1: 0; case 2: 0; i=2; } i; })");
  assert(0, ({ int i=0; switch(3) {case 0: 0; case 1: 0; case 2: 0; i=2; } i; }), "({ int i=0; switch(3) {case 0: 0; case 1: 0; case 2: 0; i=2; } i; })");

  assert(1, 1 << 0, "1 << 0");
  assert(8, 1 << 3, "1 << 3");
  assert(1, 8 >> 3, "8 >> 3");
  assert(2, 5 >> 1, "5 >> 1");
  assert(-1, -1 >> 1, "-1 >> 1");
  assert(1, ({ int i=1; i<<=0; i; }), "({ int i=1; i<<=0; i; })");
  assert(8, ({ int i=1; i<<=3; i; }), "({ int i=1; i<<=3; i; })");
  assert(1, ({ int i=8; i>>=3; i; }), "({ int i=8; i>>=3; i; })");
  assert(2, ({ int i=5; i>>=1; i; }), "({ int i=5; i>>=1; i; })");
  assert(-1, ({ int i=-1; i>>=1; i; }), "({ int i=-1; i>>=1; i; })");

  assert(2, 0 ? 1 : 2, "0 ?: 1 : 2");
  assert(1, 1 ? 1 : 2, "1 ?: 1 : 2");
  assert(-1, 0 ? -2 : -1, "0 ?: -2 : -1");
  assert(-2, 1 ? -2 : -1, "1 ?: -2 : -1");
  assert(4, sizeof(0 ? 1 : 2), "sizeof(0 ? 1 : 2)");
  assert(8, sizeof(0 ? (long)1 : (long)2), "sizeof(0 ? (long)1 : (long)2)");
  assert(8, sizeof(0 ? 1 : (long)2), "sizeof(0 ? 1 : (long)2)");
  assert(8, sizeof(0 ? (long)1 : 2), "sizeof(0 ? (long)1 : 2)");

  1 ? -2 : (void)-1;

  assert(10, ({ enum { ten=1+2+3+4 }; ten; }), "({ enum { ten=1+2+3+4 }; ten; })");
  assert(1, ({ int i=0; switch(3){case 5-2+0*3: i++; } i; }), "({ int i=0; switch(3){case 5-2+0*3: i++; } i; })");
  assert(8, ({ int x[1+1]; sizeof(x); }), "({ int x[1+1]; sizeof(x); })");
  assert(4, ({ int x[2-1]; sizeof(x); }), "({ int x[2-1]; sizeof(x); })");
  assert(32, ({ int x[4*2]; sizeof(x); }), "({ int x[4*2]; sizeof(x); })");
  assert(12, ({ int x[6/2]; sizeof(x); }), "({ int x[6/2]; sizeof(x); })");
  assert(8, ({ int x[0b11&0b10]; sizeof(x); }), "({ int x[0b11&0b10]; sizeof(x); })");
  assert(12, ({ int x[0b01|0b10]; sizeof(x); }), "({ int x[0b01|0b10]; sizeof(x); })");
  assert(12, ({ int x[0b01^0b10]; sizeof(x); }), "({ int x[0b01^0b10]; sizeof(x); })");
  assert(16, ({ int x[1<<2]; sizeof(x); }), "({ int x[1<<2]; sizeof(x); })");
  assert(8, ({ int x[5>>1]; sizeof(x); }), "({ int x[5>>1]; sizeof(x); })");
  assert(8, ({ int x[(1==1)+1]; sizeof(x); }), "({ int x[(1==1)+1]; sizeof(x); })");
  assert(4, ({ int x[(1!=1)+1]; sizeof(x); }), "({ int x[(1!=1)+1]; sizeof(x); })");
  assert(8, ({ int x[(1<2)+1]; sizeof(x); }), "({ int x[(1<2)+1]; sizeof(x); })");
  assert(8, ({ int x[(1<=2)+1]; sizeof(x); }), "({ int x[(1<=2)+1]; sizeof(x); })");
  assert(4, ({ int x[(1>2)+1]; sizeof(x); }), "({ int x[(1>2)+1]; sizeof(x); })");
  assert(4, ({ int x[(1>=2)+1]; sizeof(x); }), "({ int x[(1>=2)+1]; sizeof(x); })");
  assert(8, ({ int x[1?2:3]; sizeof(x); }), "({ int x[1?2:3]; sizeof(x); })");
  assert(2, ({ char x[(1,2)]; sizeof(x); }), "({ int x[(1,2)]; sizeof(x); })");
  assert(1, ({ char x[!0]; sizeof(x); }), "({ int x[!0]; sizeof(x); })");
  assert(2, ({ char x[~-3]; sizeof(x); }), "({ int x[~-3]; sizeof(x); })");
  assert(3, ({ char x[(1||1) * 2 + 1]; sizeof(x); }), "({ char x[(1||1) * 2 + 1]; sizeof(x); })");
  assert(3, ({ char x[(0||1) * 2 + 1]; sizeof(x); }), "({ char x[(0||1) * 2 + 1]; sizeof(x); })");
  assert(1, ({ char x[(0||0) * 2 + 1]; sizeof(x); }), "({ char x[(0||0) * 2 + 1]; sizeof(x); })");
  assert(3, ({ char x[(1&&1) * 2 + 1]; sizeof(x); }), "({ char x[(1&&1) * 2 + 1]; sizeof(x); })");
  assert(1, ({ char x[(1&&0) * 2 + 1]; sizeof(x); }), "({ char x[(1&&0) * 2 + 1]; sizeof(x); })");
  assert(1, ({ char x[(char)257]; sizeof(x); }), "({ char x[(char)257]; sizeof(x); })");
  assert(0x10f, ({ char x[(short)0xffff010f]; sizeof(x); }), "({ char x[(short)0xffff010f]; sizeof(x); })");
  assert(4, ({ char x[(int)0xfffffffffff + 5]; sizeof(x); }), "({ char x[(int)0xfffffffffff + 5]; sizeof(x); })");

  assert(4, ({ int x[2] = {4,5}; x[0]; }), "({ int x[2] = {4,5}; x[0]; })");
  assert(5, ({ int x[2] = {4,5}; x[1]; }), "({ int x[2] = {4,5}; x[1]; })");
  assert(6, ({ int x[2][2] = {{6,7},{8,9}}; x[0][0]; }), "({ int x[2][2] = {{6,7},{8,9}}; x[0][0]; })");
  assert(7, ({ int x[2][2] = {{6,7},{8,9}}; x[0][1]; }), "({ int x[2][2] = {{6,7},{8,9}}; x[0][1]; })");
  assert(8, ({ int x[2][2] = {{6,7},{8,9}}; x[1][0]; }), "({ int x[2][2] = {{6,7},{8,9}}; x[1][0]; })");
  assert(9, ({ int x[2][2] = {{6,7},{8,9}}; x[1][1]; }), "({ int x[2][2] = {{6,7},{8,9}}; x[1][1]; })");

  assert(0, ({ int x[3]={}; x[0]; }), "({ int x[3]={}; x[0]; })");
  assert(1, ({ int x[3]={1}; x[0]; }), "({ int x[3]={1}; x[0]; })");
  assert(0, ({ int x[3]={1}; x[1]; }), "({ int x[3]={1}; x[1]; })");
  assert(6, ({ int x[2][2] = {{6,7}}; x[0][0]; }), "({ int x[2][2] = {{6,7}}; x[0][0]; })");
  assert(7, ({ int x[2][2] = {{6,7}}; x[0][1]; }), "({ int x[2][2] = {{6,7}}; x[0][1]; })");
  assert(0, ({ int x[2][2] = {{6,7}}; x[1][0]; }), "({ int x[2][2] = {{6,7}}; x[1][0]; })");
  assert(0, ({ int x[2][2] = {{6,7}}; x[1][1]; }), "({ int x[2][2] = {{6,7}}; x[1][1]; })");
  assert(6, ({ int x[2][2] = {{6,7},{8}}; x[0][0]; }), "({ int x[2][2] = {{6,7},{8}}; x[0][0]; })");
  assert(7, ({ int x[2][2] = {{6,7},{8}}; x[0][1]; }), "({ int x[2][2] = {{6,7},{8}}; x[0][1]; })");
  assert(8, ({ int x[2][2] = {{6,7},{8}}; x[1][0]; }), "({ int x[2][2] = {{6,7},{8}}; x[1][0]; })");
  assert(0, ({ int x[2][2] = {{6,7},{8}}; x[1][1]; }), "({ int x[2][2] = {{6,7},{8}}; x[1][1]; })");

  assert('a', ({ char x[4]="abc"; x[0]; }), "({ char x[4]=\"abc\"; x[0]; })");
  assert('b', ({ char x[4]="abc"; x[1]; }), "({ char x[4]=\"abc\"; x[1]; })");
  assert('c', ({ char x[4]="abc"; x[2]; }), "({ char x[4]=\"abc\"; x[2]; })");
  assert('a', ({ char x[4]="ab"; x[0]; }), "({ char x[4]=\"ab\"; x[0]; })");
  assert('b', ({ char x[4]="ab"; x[1]; }), "({ char x[4]=\"ab\"; x[1]; })");
  assert('a', ({ char x[2][4] = {"abc","def"}; x[0][0]; }), "({ int x[2][4] = {{\"abc\",\"def\"}}; x[0][0]; })");
  assert('e', ({ char x[2][4] = {"abc","def"}; x[1][1]; }), "({ int x[2][4] = {{\"abc\",\"def\"}}; x[1][1]; })");
  assert('f', ({ char x[2][4] = {"abc","def"}; x[1][2]; }), "({ int x[2][4] = {{\"abc\",\"def\"}}; x[1][2]; })");

  assert(4, ({ int x[] = {3,4}; x[1]; }), "({ int x[] = {3,4}; x[1]; })");
  assert(6, ({ int x[] = {3,4,5,6}; x[3]; }), "({ int x[] = {3,4,5,6}; x[3]; }))");
  assert(16, ({ int x[] = {3,4,5,6}; sizeof(x); }), "({ int x[] = {3,4,5,6}; sizeof(x); })");
  assert(4, ({ char x[] = "foo"; sizeof(x); }), "({ char x[] = \"foo\"; sizeof(x); })");

  assert(1, ({ struct {int a; int b; int c; } x={1,2,3}; x.a; }), "({ struct {int a; int b; int c; } x={1,2,3}; x.a; })");
  assert(2, ({ struct {int a; int b; int c; } x={1,2,3}; x.b; }), "({ struct {int a; int b; int c; } x={1,2,3}; x.b; })");
  assert(3, ({ struct {int a; int b; int c; } x={1,2,3}; x.c; }), "({ struct {int a; int b; int c; } x={1,2,3}; x.c; })");
  assert(0, ({ struct {int a; int b; int c; } x={1}; x.b; }), "({ struct {int a; int b; int c; } x={1}; x.b; })");
  assert(0, ({ struct {int a; int b; int c; } x={1}; x.c; }), "({ struct {int a; int b; int c; } x={1}; x.c; })");
  assert(0, ({ struct {int a; int b; int c; } x={0}; x.a; }), "({ struct {int a; int b; int c; } x={0}; x.a; })");

  assert(1, ({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[0].a; }), "({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[0].a; })");
  assert(2, ({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[0].b; }), "({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[0].b; })");
  assert(3, ({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[1].a; }), "({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[1].a; })");
  assert(4, ({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[1].b; }), "({ struct {int a,b;} x[2]={{1,2},{3,4}}; x[1].b; })");

  assert(1, ({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; x.a; }),
         "({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; x.a; })");
  assert(2, ({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; x.b; }),
         "({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; x.b; })");
  assert(3, ({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; x.c; }),
         "({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; x.c; })");
  assert(1, ({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; T y=x; y.a; }),
         "({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; T y=x; y.a; })");
  assert(2, ({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; T y=x; y.b; }),
         "({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; T y=x; y.b; })");
  assert(3, ({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; T y=x; y.c; }),
         "({ typedef struct {int a; int b; int c; } T; T x={1,2,3}; T y=x; y.c; })");
  assert(5, g5, "g5");
  assert(6, g6, "g6");
  assert(7, g7, "g7");
  assert(8, g8, "g8");
  assert(97, g9[0], "g9[0]");
  assert(98, g9[1], "g9[1]");
  assert(99, g9[2], "g9[2]");

  assert(0, g10[0], "g10[0]");
  assert(1, g10[1], "g10[1]");
  assert(2, g10[2], "g10[2]");
  assert(1, g11[0].a, "g11[0].a");
  assert(2, g11[0].b, "g11[0].b");
  assert(3, g11[1].a, "g11[1].a");
  assert(4, g11[1].b, "g11[1].b");
  assert(1, g12[0].a[0], "g12[0].a[0]");
  assert(2, g12[0].a[1], "g12[0].a[1]");
  assert(0, g12[1].a[0], "g12[1].a[0]");
  assert(0, g12[1].a[1], "g12[1].a[1]");

  assert(7, sizeof(g13), "sizeof(g13)");
  assert(10, sizeof(g14), "sizeof(g14)");
  assert(3, sizeof(g15), "sizeof(g15)");
  assert(0, memcmp(g13, "foobar", 7), "memcmp(g13, \"foobar\", 7)");
  assert(0, memcmp(g14, "foobar\0\0\0", 10), "memcmp(g14, \"foobar\0\0\0\", 10)");
  assert(0, memcmp(g15, "foo", 3), "memcmp(g15, \"foo\", 3)");
  assert(0, strcmp(g16, "bar"), "strcmp(g16, \"bar\")");
  assert(0, strcmp(g17, "foobar"), "strcmp(g17, \"foobar\")");
  assert(0, strcmp(g18 + 3, "foobar"), "strcmp(g18 + 3, \"foobar\")");
  assert(5, *g21, "*g21");
  assert(4, *g23, "*g23");
  assert(4, **g24, "**g24");
  assert(3, *g25, "g25");
  assert(4, *g27, "*g27");
  assert(3, *g28[1], "*g28[1]");

  assert(1, g29[0].a[0], "g29[0].a[0]");
  assert(2, g29[0].a[1], "g29[0].a[1]");
  assert(3, g29[1].a[0], "g29[1].a[0]");
  assert(4, g29[1].a[1], "g29[1].a[1]");
  assert(1, g30[0].a[0], "g30[0].a[0]");
  assert(2, g30[0].a[1], "g30[0].a[1]");
  assert(3, g30[1].a[0], "g30[1].a[0]");
  assert(4, g30[1].a[1], "g30[1].a[1]");
  assert(0, strcmp(g31[0], "foo"), "strcmp(g31[0], \"foo\")");
  assert(0, strcmp(g31[1], "bar"), "strcmp(g31[1], \"bar\")");

  assert(0, ({ char *str={"foo"}; strcmp(str, "foo"); }), "({ char *str={\"foo\"}; strcmp(str, \"foo\"); })");
  assert(0, strcmp(g32, "foobar"), "strcmp(g32, \"foobar\")");

  assert(2, ({ enum {a, b, c,}; c; }), "({ enum {a, b, c,}; c; })");
  assert(3, ({ int x[3]={1,2,3,}; x[2]; }), "({ int x[3]={1,2,3,}; x[2]; })");
  assert(5, ({ struct {int a,b; char c; } x ={1,3,5}; x.c; }), "({ struct {int a,b; char c; } x ={1,3,5}; x.c; })");

  assert(3, ext1, "ext1");
  ext2 = &ext1;
  assert(3, *ext2, "*ext2");

  assert(6, ({ Ty1 x = {2, 6}; x.b; }), "({ Ty1 x = {2, 6}; x.b; })");

  assert(1, _Alignof(char), "_Alignof(char)");
  assert(2, _Alignof(short), "_Alignof(short)");
  assert(4, _Alignof(int), "_Alignof(int)");
  assert(8, _Alignof(long), "_Alignof(long)");
  assert(8, _Alignof(long long), "_Alignof(char)");
  assert(1, _Alignof(char[56]), "_Alignof(char[56])");
  assert(4, _Alignof(int[3]), "_Alignof(int[3])");
  assert(1, _Alignof(struct {char a; char b; }[2]), "_Alignof(struct {char a; char b; }[2]");
  assert(8, _Alignof(struct {char a; long b; }[2]), "_Alignof(struct {char a; char b; }[2]");

  assert(0, (long)(char *)&g_aligned1 % 512, "(long)(char *)&g_aligned1 % 512");
  assert(0, (long)(char *)&g_aligned2 % 512, "(long)(char *)&g_aligned2 % 512");
  assert(0, (long)(char *)&g_aligned3 % 128, "(long)(char *)&g_aligned3 % 128");
  assert(1, ({ _Alignas(char) char x, y; &y - &x; }), "({ _Alignas(char) char x, y; &y - &x; })");
  assert(2, ({ _Alignas(short) char x, y; &y - &x; }), "({ _Alignas(short) char x, y; &y - &x; })");
  assert(64, ({ _Alignas(64) char x, y; &y - &x; }), "({ _Alignas(64) char x, y; &y - &x; })");
  assert(16, ({ struct { _Alignas(16) char x, y; } a; &a.y - &a.x; }), "({ struct { _Alignas(16) char x, y; } a; &a.y - &a.x; })");
  assert(9, ({ struct { _Alignas(16) char x; _Alignas(8) char y; char z; } a; &a.z - &a.x; }), "({ struct { _Alignas(16) char x; _Alignas(8) char y; char z; } a; &a.z - &a.x; })");
  assert(16, ({ struct { _Alignas(16) char x; _Alignas(8) char y; char z; } a; sizeof(a); }), "({ struct { _Alignas(16) char x; _Alignas(8) char y; char z; } sizeof(a); })");
  assert(16, ({ struct T { _Alignas(16) char x, y; }; _Alignof(struct T); }), "({ struct T { _Alignas(16) char x, y; } _Alignof(struct T); })");

  assert(5, counter(), "counter()");
  assert(6, counter(), "counter()");
  assert(7, counter(), "counter()");

  assert(1, (int){1}, "(int){1}");
  assert(2, ((int[]){1, 2})[1], "((int[]){1, 2})[1]");
  assert('a', ((struct {char a; int b; }){'a', 3}).a, "((struct {char a; int b; }){'a', 3}).a");
  assert(5, ({ int x=5; (int){x}; }), "({ int x=5; (int){x}; })");

  assert(1, tree->val, "tree->val");
  assert(2, tree->lhs->val, "tree->lhs->val");
  assert(3, tree->lhs->lhs->val, "tree->lhs->lhs->val");
  assert(4, tree->lhs->rhs->val, "tree->lhs->rhs->val");

  assert(3, ext3, "ext3");

  assert(0, false_fn(), "false_fn()");
  assert(1, true_fn(), "true_fn()");

  ret_none();

  assert(6, add_all1(1, 2, 3, 0), "add_all1(1, 2, 3, 0)");
  assert(10, add_all1(1, 2, 3, 4, 0), "add_all1(1, 2, 3, 4, 0)");
  assert(15, add_all1(1, 2, 3, 4, 5, 0), "add_all1(1, 2, 3, 4, 5, 0)");

  assert(6, add_all3(1, 2, 3, 0), "add_all3(1, 2, 3, 0)");
  assert(10, add_all3(1, 2, 3, 4, 0), "add_all3(1, 2, 3, 4, 0)");
  assert(15, add_all3(1, 2, 3, 4, 5, 0), "add_all3(1, 2, 3, 4, 5, 0)");
  assert(0, ({ char buf[100]; sprintf(buf, "%d %d %s", 1, 2, "foo"); strcmp("1 2 foo", buf); }), "({ char buf[100]; sprintf(buf, \"%d %d %s\", 1, 2, \"foo\"); strcmp(\"1 2 foo\", buf); })");
  assert(0, ({ char buf[100]; fmt(buf, "%d %d %s", 1, 2, "foo"); strcmp("1 2 foo", buf); }), "({ char buf[100]; fmt(buf, \"%d %d %s\", 1, 2, \"foo\"); strcmp(\"1 2 foo\", buf); })");

  assert(15, ({ int i=1, sum=0; do {sum+=i;} while (i++<5); sum; }), "({ int i=1, sum=0; do {sum+=i;} while (i++<5); sum; })");
  assert(10, ({ int i=1, sum=0; do {sum+=i; if (sum==10) break; continue; sum*=100; } while (i++<5); sum; }), "({ int i=1, sum=0; do {sum+=i; if (sum==10) break; continue; sum*=100; } while (i++<5); sum; })");

  // #include directive
  assert(17, g33 + ret7(), "g33 + ret7()");
  assert(8, ret8(), "ret8()");

  // Object-like macro
  assert(2020, ({MY_INT x = M1; x; }), "({MY_INT x = M1; x;})");
  assert(3, ret3(), "ret3()");
  int M2 = 6;
  #define M2 M2 + 3
  assert(9, M2, "M2");

  #define M3 M2 + 3
  assert(12, M3, "M3");

  int M4 = 3;
  #define M4 M5 * 5
  #define M5 M4 + 2
  // M4 -> M5 * 5      -> M4 + 2 * 5   -> 3 + 2 * 5
  //    ^ M4 exppand   ^ M5 expand       ^ M4 = 3
  assert(13, M4, "M4");

  assert(4, ({ sizeof(signed); }), "({ sizeof(signed); })");
  assert(1, ({ sizeof(signed char); }), "({ sizeof(signed char); })");
  assert(2, ({ sizeof(signed short); }), "({ sizeof(signed short); })");
  assert(2, ({ sizeof(signed short int); }), "({ sizeof(signed short int); })");
  assert(2, ({ sizeof(signed int short); }), "({ sizeof(signed int short); })");
  assert(4, ({ sizeof(signed int); }), "({ sizeof(signed int); })");
  assert(8, ({ sizeof(signed long); }), "({ sizeof(signed long); })");
  assert(8, ({ sizeof(signed long int); }), "({ sizeof(signed long int); })");
  assert(8, ({ sizeof(signed int long); }), "({ sizeof(signed int long); })");
  assert(8, ({ sizeof(signed char *); }), "({ sizeof(csigned har *); })");
  assert(8, ({ sizeof(signed int *); }), "({ sizeof(signed int *); })");
  assert(8, ({ sizeof(signed long *); }), "({ sizeof(signed long *); })");
  assert(8, ({ sizeof(signed int **); }), "({ sizeof(signed int **); })");
  assert(8, ({ sizeof(signed int(*)[4]); }), "({ sizeof(signed int(*)[4]); })");
  assert(16, ({ sizeof(signed int[4]); }), "({ sizeof(signed int[4]); })");
  assert(24, ({ sizeof(signed int[2][3]); }), "({ sizeof(signed int[2][3]); })");
  assert(8, ({ sizeof(struct { signed int a; signed int b; }); }), "({ sizeof(struct { signed int a; signed int b; }); })");
  assert(8, sizeof(signed long long), "sizeof(signed long long)");
  assert(8, sizeof(signed long long int), "sizeof(signed long long int)");
  assert(1, sizeof(signed char signed), "sizeof(signed char signed)");

  assert(4, ({ sizeof(unsigned); }), "({ sizeof(unsigned); })");
  assert(1, ({ sizeof(unsigned char); }), "({ sizeof(unsigned char); })");
  assert(2, ({ sizeof(unsigned short); }), "({ sizeof(unsigned short); })");
  assert(2, ({ sizeof(unsigned short int); }), "({ sizeof(unsigned short int); })");
  assert(2, ({ sizeof(unsigned int short); }), "({ sizeof(unsigned int short); })");
  assert(4, ({ sizeof(unsigned int); }), "({ sizeof(unsigned int); })");
  assert(8, ({ sizeof(unsigned long); }), "({ sizeof(unsigned long); })");
  assert(8, ({ sizeof(unsigned long int); }), "({ sizeof(unsigned long int); })");
  assert(8, ({ sizeof(unsigned int long); }), "({ sizeof(unsigned int long); })");
  assert(8, ({ sizeof(unsigned char *); }), "({ sizeof(unsigned har *); })");
  assert(8, ({ sizeof(unsigned int *); }), "({ sizeof(unsigned int *); })");
  assert(8, ({ sizeof(unsigned long *); }), "({ sizeof(unsigned long *); })");
  assert(8, ({ sizeof(unsigned int **); }), "({ sizeof(unsigned int **); })");
  assert(8, ({ sizeof(unsigned int(*)[4]); }), "({ sizeof(unsigned int(*)[4]); })");
  assert(16, ({ sizeof(unsigned int[4]); }), "({ sizeof(unsigned int[4]); })");
  assert(24, ({ sizeof(unsigned int[2][3]); }), "({ sizeof(unsigned int[2][3]); })");
  assert(8, ({ sizeof(struct { unsigned int a; unsigned int b; }); }), "({ sizeof(struct { unsigned int a; unsigned int b; }); })");
  assert(1, sizeof(unsigned char), "sizeof(unsigned char)");
  assert(1, sizeof(unsigned char unsigned), "sizeof(unsigned char unsigned)");
  assert(8, sizeof(unsigned long long), "sizeof(unsigned long long)");
  assert(8, sizeof(unsigned long long int), "sizeof(unsigned long long int)");
  assert(-1, (char)255, "(char)255");
  assert(-2, (char)254, "(char)254");
  assert(-1, (signed char)255, "(signed char)255");
  assert(255, (unsigned char)255, "(unsigned char)255");
  assert(-1, (short)0xffff, "(short)0xffff");
  assert(0xffff, (unsigned short)0xffff, "(unsigned short)0xffff");
  assert(-1, (signed)0xffffffff, "(signed)0xffffffff");
  assert(0xffffffff, (unsigned)0xffffffff, "(unsigned)0xffffffff");
  assert(-1, (int)(unsigned int)(-1), "(int)(unsigned int)(-1)");

  assert(4, sizeof((char)12 + (char)12), "sizeof((char)12 + (char)12)");
  assert(4, sizeof((short)12 + (short)12), "sizeof((short)12 + (short)12)");
  assert(4, sizeof(1 ? 2 : 3), "sizeof(true ? 2 : 3)");
  assert(4, sizeof((char)12 + (short)12), "sizeof(char)12 + (short)12)");
  assert(4, sizeof((char)12 + (int)12), "sizeof(char)12 + (int)12)");
  assert(8, sizeof((char)12 + (long)12), "sizeof((char)12 + (long)12)");

  assert(5, ({ unsigned char x = 5; x;}), "({ unsigned char x = 5; x;})");
  assert(5, ({ unsigned short x = 5; x;}), "({ unsigned short x = 5; x;})");
  assert(5, ({ unsigned x = 5; x;}), "({ unsigned char x = 5; x;})");
  assert(5, ({ unsigned long x = 5; x;}), "({ unsigned long x = 5; x;})");
  
  assert(0, -1 < (unsigned)1, "-1 < (unsigned)1");
  assert(-1, (unsigned long)-1, "(unsigned long)-1");
  assert(0x7f, ((unsigned char)-1) >> 1, "((unsigned char)-1) >> 1");
  assert(0x7fff, ((unsigned short)-1) >> 1, "((unsigned shor)-1) >> 1");
  assert(0x7fffffff, ((unsigned)-1) >> 1, "((unsigned)-1) >> 1");
  assert(0x7fffffffffffffff, ((unsigned long)-1) >> 1, "((unsigned long)-1) >> 1");
  assert(0x1fe, ((unsigned char)-1) << 1, "((unsigned char)-1) << 1");
  assert(0x1fffe, ((unsigned short)-1) << 1, "((unsigned shor)-1) << 1");
  assert(0xfffffffe, ((unsigned)-1) << 1, "((unsigned)-1) << 1");
  assert(0xfffffffffffffffe, ((unsigned long)-1) << 1, "((unsigned long)-1) << 1");

  assert(5, (unsigned char)10 / (unsigned char)2, "(unsigned char)10 / (unsigned char)2");
  assert(5, (unsigned short)10 / (unsigned short)2, "(unsigned short)10 / (unsigned short)2");
  assert(5, (unsigned)10 / (unsigned)2, "(unsigned)10 / (unsigned)2");
  assert(5, (unsigned long)10 / (unsigned long)2, "(unsigned long)10 / (unsigned long)2");
  assert(1, (unsigned char)11 % (unsigned char)2, "(unsigned char)11 %% (unsigned char)2");
  assert(1, (unsigned short)11 % (unsigned short)2, "(unsigned short)11 %% (unsigned short)2");
  assert(1, (unsigned)11 % (unsigned)2, "(unsigned)11 %% (unsigned)2");
  assert(1, (unsigned long)11 % (unsigned long)2, "(unsigned long)11 %% (unsigned long)2");
  assert(5, 10 / (unsigned char)2, "10 / (unsigned char)2");
  assert(5, 10 / (unsigned short)2, "10 / (unsigned short)2");
  assert(5, 10 / (unsigned)2, "10 / (unsigned)2");
  assert(5, 10 / (unsigned long)2, "10 / (unsigned long)2");
  assert(1, 11 % (unsigned char)2, "11 %% (unsigned char)2");
  assert(1, 11 % (unsigned short)2, "11 %% (unsigned short)2");
  assert(1, 11 % (unsigned)2, "11 %% (unsigned)2");
  assert(1, 11 % (unsigned long)2, "11 %% (unsigned long)2");
  assert(5, (unsigned char)10 / 2, "(unsigned char)10 / 2");
  assert(5, (unsigned short)10 / 2, "(unsigned short)10 / 2");
  assert(5, (unsigned)10 / 2, "(unsigned)10 / 2");
  assert(5, (unsigned long)10 / 2, "(unsigned long)10 / 2");
  assert(1, (unsigned char)11 % 2, "(unsigned char)11 %% 2");
  assert(1, (unsigned short)11 % 2, "(unsigned short)11 %% 2");
  assert(1, (unsigned)11 % 2, "(unsigned)11 %% 2");
  assert(1, (unsigned long)11 % 2, "(unsigned long)11 %% 2");
  assert(-1, ({ short x = 0xffff; (int)x; }), "({ short x = 0xffff; (int)x; })");
  assert(0xffff, ({ unsigned short x = 0xffff; (int)x; }), "({ unsigned short x = 0xffff; (int)x; })");
  assert(-1, ({ typedef short T; T x = 0xffff; (int)x; }), "({unsigned T; T x = 0xffff; (int)x; })");
  assert(0xffff, ({ typedef unsigned short T; T x = 0xffff; (int)x; }), "({unsigned short T; T x = 0xffff; (int)x; })");

  assert(4, sizeof(2147483647), "sizeof(2147483647)"); //1 << 31 - 1 = 2147483647, int
  assert(8, sizeof(2147483648), "sizeof(2147483647)"); //1 << 31     = 2147483648, long
  assert(8, sizeof(4611686018427387904), "sizeof(4611686018427387904)"); //1 << 63 - 1 = 4611686018427387904, long
  // assert(16, sizeof(9223372036854775808), "sizeof(9223372036854775808)"); //1 << 63 = 9223372036854775808 __int128_t

  assert(4, sizeof(0xfffffffe), "sizeof(0xffffffff)"); // unsigned int
  assert(4, sizeof(0x7fffffff), "sizeof(0x7fffffff)"); // int
  assert(8, sizeof(0x100000000), "sizeof(0x100000000)"); // long
  assert(8, sizeof(0x3fffffffffffffff), "sizeof(0x3fffffffffffffff)"); // long
  assert(8, sizeof(0x7fffffffffffffff), "sizeof(0x7fffffffffffffff)"); // long
  assert(8, sizeof(0x8000000000000000), "sizeof(0x8000000000000000)"); // unsigned long

  assert(4, sizeof(0), "sizeof(0)");
  assert(4, sizeof(0U), "sizeof(0U)");
  assert(8, sizeof(0L), "sizeof(0L)");
  assert(4, sizeof(0u), "sizeof(0u)");
  assert(8, sizeof(0l), "sizeof(0l)");
  assert(8, sizeof(0UL), "sizeof(0UL)");
  assert(8, sizeof(0uL), "sizeof(0uL)");
  assert(8, sizeof(0Ul), "sizeof(0Ul)");
  assert(8, sizeof(0ul), "sizeof(0ul)");
  assert(8, sizeof(0LLU), "sizeof(0LLU)");
  assert(8, sizeof(0LLu), "sizeof(LLu)");
  assert(8, sizeof(0ULL), "sizeof(ULL)");
  assert(8, sizeof(0Ull), "sizeof(0Ull)");
  assert(8, sizeof(0uLL), "sizeof(0uLL)");
  assert(8, sizeof(0ull), "sizeof(0ull)");

  assert(4, sizeof(10), "sizeof(10)");
  assert(4, sizeof(10U), "sizeof(10U)");
  assert(8, sizeof(10L), "sizeof(10L)");
  assert(4, sizeof(10u), "sizeof(10u)");
  assert(8, sizeof(10l), "sizeof(10l)");
  assert(8, sizeof(10UL), "sizeof(10UL)");
  assert(8, sizeof(10uL), "sizeof(10uL)");
  assert(8, sizeof(10Ul), "sizeof(10Ul)");
  assert(8, sizeof(10ul), "sizeof(10ul)");
  assert(8, sizeof(10LLU), "sizeof(10LLU)");
  assert(8, sizeof(10LLu), "sizeof(LLu)");
  assert(8, sizeof(10ULL), "sizeof(ULL)");
  assert(8, sizeof(10Ull), "sizeof(10Ull)");
  assert(8, sizeof(10uLL), "sizeof(10uLL)");
  assert(8, sizeof(10ull), "sizeof(10ull)");

  assert(-1, (int)(4294967295U), "(int)(4294967296U)"); // (1 << 32) - 1
  assert(-1, (long)(18446744073709551615UL), "(long)(18446744073709551615UL)"); //  (1 << 64) - 1
  assert(1, 9223372036854775808 >> 63  /* (1 << 63) >> 63 */, "9223372036854775808 >> 63");

  { const int x; }
  { const x; }
  { int const x; }
  { const int const const x; }
  assert(5, ({ const int  x = 5; x; }), "({ const int x = 5; x; })");
  assert(5, ({ const int x = 5; int * const y = &x; *y; }), "({ const int x = 5; int * const y = &x; *y; })");
  assert(5, ({ const int x = 5; const int * const y = &x; *y; }), "({ const int x = 5; const int * const y = &x; *y; })");
  assert(5, ({ const int x = 5; const int * const const y = &x; *y; }), "({ const int x = 5; const int * const const y = &x; *y; })");

  assert(5, const_arg_func(5), "const_arg_func(5)");
  assert(5, const_rest_func(5), "const_rest_func(5)");

  assert(8, sizeof(_Alignof(char)), "sizeof(_Alignof(char))");
  assert(1, sizeof(char) << 63 >> 63, "sizeof(char) << 31 >> 31");
  assert(1, _Alignof(char) << 63 >> 63, "_Alignof(char) << 63 >> 63");

  assert(9, ({ char x[(char)0xffffffff + 10]; sizeof(x); }), "({ char x[(char)0xffffffff + 10]; sizeof(x); })");
  assert(255, ({ char x[(unsigned char)0xffffffff]; sizeof(x); }), "({ char x[(unsigned char)0xffffffff]; sizeof(x); })");
  assert(0xffff, ({ char x[(unsigned short)0xffffffff]; sizeof(x); }), "({ char x[(unsigned short)0xffffffff]; sizeof(x); })");
  assert(1, ({ char x[(unsigned) 0xffffffff >> 31]; sizeof(x); }), "({ char x[(unsigned) 0xffffffff >> 31]; sizeof(x); })");
  assert(1, ({ char x[(unsigned long) 0xffffffffffffffff >> 63]; sizeof(x); }), "({ char x[(unsigned long) 0xffffffffffffffff >> 63]; sizeof(x); })");

  { volatile x; }
  { int volatile x; }
  { volatile int x; }
  { volatile int volatile volatile x; }
  { volatile int * volatile volatile x; }
  { int ** restrict const volatile *x; }
  { register int x = 1; }

  { typedef int ;}

  0.0;
  1.0;
  3e+8;
  0x10.1p13;
  .1E4;
  .1E4f;
  1.4f;
  assert(4, sizeof(1.2f), "sizeof(1.2f)");
  assert(4, sizeof(1.2F), "sizeof(1.2F)");
  assert(8, sizeof(1.2), "sizeof(1.2)");
  assert(8, sizeof(1.2l), "sizeof(1.2l)");
  assert(8, sizeof(1.2L), "sizeof(1.2L)");
  assert(8, sizeof(.0), "sizeof(.0)");
  assert(8, sizeof(0.), "sizeof(0.)");
  assert(8, sizeof((double)1), "sizeof((double)1)");
  assert(4, sizeof((float)1), "sizeof((float)1)");
  assert(8, sizeof((double)1U), "sizeof((double)1U)");
  assert(4, sizeof((float)1U), "sizeof((float)1U)");
  assert(8, sizeof((double)1L), "sizeof((double)1L)");
  assert(4, sizeof((float)1L), "sizeof((float)1L)");
  assert(8, sizeof((double)34.4F), "sizeof((double)34.4F)");
  assert(4, sizeof((float)34.4), "sizeof((float)34.4)");
  assert(8, sizeof((long double)34.4F), "sizeof((long double)34.4F)");

  assert(0, (_Bool)0.0f, "(_Bool)0.0f");
  assert(1, (_Bool)0.1f, "(_Bool)0.1f");
  assert(0, (_Bool)0.0, "(_Bool)0.0");
  assert(1, (_Bool)0.1, "(_Bool)0.1");
  assert(3, (char)3.3, "(char)3.3");
  assert(3, (char)3.3f, "(char)3.3f");
  assert(3, (short)3.3, "(short)3.3");
  assert(3, (short)3.3f, "(short)3.3f");
  assert(3, (unsigned short)3.3, "(unsigned short)3.3");
  assert(3, (unsigned short)3.3f, "(unsigned short)3.3f");
  assert(3, (int)3.3, "(int)3.3");
  assert(3, (int)3.3f, "(int)3.3f");
  assert(3, (int)3.3L, "(int)3.3L");
  assert(2, (int)((double)2.3f), "(int)((double)2.3f)");
  assert(2, (int)((float)2.3), "(int)((float)2.3)");

  assert(3, ({ float x = 3.3; (int)x; }), "({ float x = 3.3; (int)x; })");
  assert(3, ({ float x = 3.3f; (int)x; }), "({ float x = 3.3f; (int)x; })");
  assert(3, ({ double x = 3.3f; (int)x; }), "({ double x = 3.3f; (int)x; })");
  assert(3, ({ double x = 3.3f; (int)x; }), "({ double x = 3.3f; (int)x; })");
  assert(3, ({ long double x = 3.3f; (int)x; }), "({ long double x = 3.3f; (int)x; })");
  assert(3, ({ long double x = 3.3f; (int)x; }), "({ long double x = 3.3f; (int)x; })");

  assert(1, 2e3f==2e3f, "2e3f==2e3f");
  assert(0, 2e3f==2e4f, "2e3f==2e4f");
  assert(1, 2.0f==2f, "2.0f==2ƒ");
  assert(1, 1.1f < 1.2f, "1.1f < 1.2ƒ");
  assert(0, 1.1f < 1.0f, "1,1f < 1.0f");
  assert(1, 1.3F <= 1.3F, "1.3F <= 1.3F");
  assert(1, 1.1F <= 1.2F, "1.1F < 1.2F");
  assert(0, 1.1F > 1.2F, "1.1F > 1.2F");
  assert(1, 1.1F > 1.0F, "1,1F > 1.0F");
  assert(1, 1.3F >= 1.3F, "1.3F >= 1.3F");
  assert(0, 1.1F >= 1.2F, "1.1F > 1.2F");

  assert(1, 2e3==2e3, "2e3==2e3");
  assert(0, 2e3==2e4, "2e3==2e4");
  assert(1, 2.0==2, "2.0==2");
  assert(1, 1.1 < 1.2, "1.1 < 1.2");
  assert(0, 1.1 < 1.0, "1,1 < 1.0");
  assert(1, 1.3 <= 1.3, "1.3 <= 1.3");
  assert(1, 1.1 <= 1.2, "1.1 < 1.2");
  assert(0, 1.1 > 1.2, "1.1 > 1.2");
  assert(1, 1.1 > 1.0, "1,1 > 1.0");
  assert(1, 1.3 >= 1.3, "1.3 >= 1.3");
  assert(0, 1.1 >= 1.2, "1.1 > 1.2");

  assert(1, 2e3l==2e3l, "2e3l==2e3l");
  assert(0, 2e3l==2e4l, "2e3l==2e4l");
  assert(1, 2.0l==2l, "2.0l==2l");
  assert(1, 1.1l < 1.2l, "1.1l < 1.2l");
  assert(0, 1.1l < 1.0l, "1,1l < 1.0l");
  assert(1, 1.3L <= 1.3L, "1.3L <= 1.3L");
  assert(1, 1.1L <= 1.2L, "1.1L < 1.2L");
  assert(0, 1.1L > 1.2L, "1.1L > 1.2L");
  assert(1, 1.1L > 1.0L, "1,1L > 1.0L");
  assert(1, 1.3L >= 1.3L, "1.3L >= 1.3L");
  assert(0, 1.1L >= 1.2L, "1.1L > 1.2L");

  assert(1, 1.1f < 1.2l, "1.1f < 1.2l");
  assert(1, (float)1==(int)1.1, "(float)1==(int)1.1");
  assert(1, (double)1==(int)1.1, "(double)1==(unsigned long)1.1");

  assert(4, sizeof(1f+2), "sizeof(1f+2)");
  assert(8, sizeof(1.+2), "sizeof(1.+2)");
  assert(4, sizeof(1f-2), "sizeof(1f-2)");
  assert(8, sizeof(1.-2), "sizeof(1.-2)");
  assert(4, sizeof(1f*2), "sizeof(1f*2)");
  assert(8, sizeof(1.*2), "sizeof(1.*2)");
  assert(4, sizeof(1f/2), "sizeof(1f/2)");
  assert(8, sizeof(1./2), "sizeof(1./2)");

  assert(5, 2.3f+2.8f, "2.3f+2.8f");
  assert(-2, 2.3f-4.7f, "2.3-4.7f");
  assert(6, 2.3f*2.7f, "2.3f*2.7f");
  assert(10, 27.3f/2.7f, "27.3f/2.7f");

  assert(5, 2.3f+2.8, "2.3f+2.8");
  assert(-2, 2.3f-4.7, "2.3f-4.7");
  assert(6, 2.3f*2.7, "2.3f*2.7");
  assert(10, 27.3f/2.7, "27.3f/2.7");
  
  assert(5, 2.3+2.8f, "2.3+2.8f");
  assert(-2, 2.3-4.7f, "2.3-4.7f");
  assert(6, 2.3*2.7f, "2.3*2.7f");
  assert(10, 27.3/2.7f, "27.3/2.7f");

  assert(5, 2.3+2.8, "2.3+2.8");
  assert(-2, 2.3-4.7, "2.3-4.7");
  assert(6, 2.3*2.7, "2.3*2.7");
  assert(10, 27.3/2.7, "27.3/2.7");

  assert(3, ({ float x=3.1; (int)x; }), "({ float x=3.1; (int)x; })");
  assert(3, ({ double x=3.1; (int)x; }), "({ double x=3.1; (int)x; })");
  assert(5, ({ float x=3.1; double y=2.3; (int)(x+y); }), "({ float x=3.1; double y=2.3; (int)(x+y); })");
  assert(-1, ({ float x=3.1; double y=4.3; (int)(x-y); }), "({ float x=3.1; double y=4.3; (int)(x-y); })");
  assert(7, ({ float x=3.1; double y=2.3; (int)(x*y); }), "({ float x=3.1; double y=2.3; (int)(x*y); })");
  assert(3, ({ float x=9.1; double y=2.3; (int)(x/y); }), "({ float x=9.1; double y=2.3; (int)(x/y); })");

  printf("OK\n");
  return 0;
}