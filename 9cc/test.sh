#! /bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 5 '1+1+1+1+1;'
assert 0 '1+1+1+1+1-1-1-1-1-1;'
assert 41 '12 + 34 - 5;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 8 '-5 + 5 * 3 - 2 * 1;'
assert 2 '+1+1;'
assert 1 '2==2;'
assert 0 '1==2;'
assert 1 '2!=1;'
assert 0 '2!=2;'
assert 1 '1<2;'
assert 0 '1<1;'
assert 1 '2<=2;'
assert 0 '2<=1;'
assert 0 '1>2;'
assert 1 '2>1;'
assert 1 '2>=2;'
assert 0 '1>=2;'
assert 1 '1 + 2 * 15 / 3 + 2 - 1 == 12 * 100 / 100;'
assert 1 '1 + 2 * 15 / 3 + 2 - 1 > 12 * 100 / 100 - 1;'
assert 1 'a=1; b=2; b-a;'
assert 50 'a=1000; b=20; c=a/b;'
assert 100 'a=1000; b=20; c=a/b; (c + 50) * (10 - 9);'
assert 5 'num = 1; num * 5;'
assert 5 'num0 = 1; num1 = num0 > 0; num0 * num1 * 5;'
assert 10 '_NUM_TEST = 10;'
assert 10 'return 10;'
assert 10 'return 10; return 20;'
assert 5 'num0 = 1; num1 = num0 > 0; return num0 * num1 * 5; return 0;'
assert 5 'return -(-5);'
assert 1 'v0=1;v1=1;v2=1;v3=1;v4=1;v5=1;v6=1;v7=1;v8=1;v9=1;v10=1;return v0;'
assert 10 'if (1) return 10; return 5;'
assert 5 'if (0) return 10; else return 5;'
assert 5 'a = 5; b = 1; if (b - a < 0) return a * b; else return 0;'
assert 100 'a=10; if (a == 0) return 0; else if (a == 10) return 100; return 1;'
assert 1 'a=10; if(a==0)return 0;else if(a==10)return 1; else return 3;'
assert 3 'a=10; if(a==0)return 0;else if(a==1)return 1; else return 3;'
assert 9 'a=10; a = a - 1; return a;'
assert 2 'a=10; while(a > 2) a = a - 1; return a;'
assert 1 'a=10; while(a > 3) a = a - 3; return a;'
assert 5 'a=10; while(a > 0) if (a == 5) return a; else a = a - 1; return 100;'
assert 10 'a=0; sum=0; for (a=0;a<5;a=a+1) sum = sum+a; return sum;'
assert 10 'sum=0; for (a=0;a<5;a=a+1) sum = sum+a; return sum;'
assert 0 'for (;;) return 0;'
assert 10 'for (a=0;a<5;) a = a + 10; return a;'
assert 10 'for (a=0;;a=a+1) if (a >= 10) return a; return 0;'
assert 10 'a=0; for (;a<10;a=a+1) a; return a;'
assert 10 'a=0; for (;;a=a+1) if (a >= 10) return a; return 0;'
assert 3 'sum=0; for (a=0;a<3;a=a+1){sum = sum + a;} return sum;'
assert 33 'sum=0; a=10; for(b=0;b<3;b=b+1){sum=sum+b; sum=sum+a;} return sum;'
assert 15 'sum=0; i=1; while(i<=5){sum=sum+i; i=i+1;} return sum;'
assert 0 'for(i=0;i<5;i=i+1){} return 0;'
assert 2 'a=0; if(a==0)a=2; return a;'
assert 2 'a=0; if(a==0){a=2;} return a;'
assert 8 'a=0; if(a==0){a=2; a=a*2; a=a*2;} return a;'

echo OK