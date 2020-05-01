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

echo OK