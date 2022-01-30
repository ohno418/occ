#!/bin/bash

assert() {
  input=$1
  expected=$2

  ./occ "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual=$?

  if [ $actual = $expected ]
  then
    echo "$input => $actual"
  else
    echo "$input => expected $expected, but got $actual"
    exit 1
  fi
}

assert "int main() { return 42; }" "42"
assert "int main() { return 123; }" "123"
assert "int main() { return 42+3; }" "45"
assert "int main() { return 42  + 13 ; }" "55"
assert "int main() { return 42+3+12; }" "57"
assert "int main() { return 42+3+12+2; }" "59"
assert "int main() { return 44-2; }" "42"
assert "int main() { return 44+22-34-12; }" "20"
assert "int main() { return 2*34; }" "68"
assert "int main() { return 2*3*4; }" "24"
assert "int main() { return 2*34-3; }" "65"
assert "int main() { return 12+2*4-3; }" "17"
assert "int main() { return 12/4; }" "3"
assert "int main() { return 12/5; }" "2"
assert "int main() { return 12+4/2-3; }" "11"
assert "int main() { return 12+3*4/2-3; }" "15"
assert "int main() { return (12+3)*4; }" "60"
assert "int main() { return (12+3)*4/(1+2); }" "20"
assert "int main() { return 1; 2; return 3; }" "1"
assert "int main() { 1; return 2; return 3; }" "2"
assert "int main() { 1; 2; return 3; }" "3"
assert "int main() { int a=5; return a; }" "5"
assert "int main() { int a=5+2; return a; }" "7"
assert "int main() { int a=5; int b=123; return a; }" "5"
assert "int main() { int a=5; int b=123; return b; }" "123"
assert "int main() { int a; int b; a=b=123; return a; }" "123"
assert "int main() { int a; int b; a=b=123; return b; }" "123"
assert "int main() { int var=12; int foo=23; return var; }" "12"
assert "int main() { int var=12; int foo=23; return foo; }" "23"
assert "int main() { int a=5; a=6; return a; }" "6"
assert "int ret42() { return 42; } int main() { return ret42(); }" "42"
assert "int main() { { 1; 2; 3; } return 4; }" "4"
assert "int main() { { 1; return 2; 3; } return 4; }" "2"
assert "int main() { ; return 3; }" "3"
assert "int main() { if (0) return 12; return 23; }" "23"
assert "int main() { if (1) return 12; return 23; }" "12"
assert "int main() { if (2) return 12; return 23; }" "12"
assert "int main() { if (0) { return 12; } return 23; }" "23"
assert "int main() { if (1) { return 12; } return 23; }" "12"
assert "int main() { int cond=0; if (cond) { return 12; } return 23; }" "23"
assert "int main() { int cond=1; if (cond) { return 12; } return 23; }" "12"
assert "int main() { if (1) { if (1) return 12; return 23; } return 34; }" "12"
assert "int main() { if (1) { if (0) return 12; return 23; } return 34; }" "23"
assert "int main() { if (0) { if (0) return 12; return 23; } return 34; }" "34"
assert "int main() { if (0) return 12; else return 23; return 34; }" "23"
assert "int main() { if (1) return 12; else return 23; return 34; }" "12"
assert "int main() { if (0) { return 12; } else { return 23; } return 34; }" "23"
assert "int main() { if (1) { return 12; } else { return 23; } return 34; }" "12"
assert "int main() { int i = 3; ++i; return i; }" "4"
assert "int main() { int i = 3; ++i; ++i; return i; }" "5"
assert "int main() { int i = 3; ++i; return ++i; }" "5"
assert "int main() { int i = 3; --i; return i; }" "2"
assert "int main() { int i = 3; --i; --i; return i; }" "1"
assert "int main() { int i = 3; --i; return --i; }" "1"
assert "int main() { int i = 7; ++i, ++i; return i; }" "9"
assert "int main() { int i = 7; ++i, i = 42, ++i; return i; }" "43"

# sizeof
assert "int main() { int a; return sizeof(a); }" "8"
assert "int main() { return sizeof(int); }" "8"

echo OK
exit 0
