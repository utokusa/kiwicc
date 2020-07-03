// This is a line comment.

/*
 * This is a block comment.
 */

int printf();
int exit();
int ret_777();

int g1;
int g2[4];
char g3;
char g4[4];

typedef int MyInt, MyInt2[4];

int assert(int expected, int actual, char *code)
{
  if (expected == actual)
  {
    printf("%s => %d\n", code, actual);
  }
  else
  {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
}

int ret3() { return 3; }
int ret5() { return 5; }
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
  assert(4, ({ sizeof(sizeof(1)); }), "({ sizeof(sizeof(1)); })");
  assert(8, ({ int x = 0; sizeof(&x); }), "({ int x = 0; sizeof(&x); })");
  assert(8, ({ int x = 0; sizeof(&x + 2); }), "({ int x = 0; sizeof(&x + 2); })");
  assert(4, ({ int x = 0; sizeof(sizeof(&x)); }), "({ int x = 0; sizeof(sizeof(&x)); })");
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

  printf("OK\n");
  return 0;
}