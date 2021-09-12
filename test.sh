assert() {
  input=$1
  expected=$2

  ./occ "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual=$?

  if [ "$expected" = "$actual" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert "int main() { return 7; }" "7"
assert "int main() { return 42; }" "42"
assert "int main() { return 123; }" "123"
assert "int main() { return 2+3; }" "5"
assert "int main() { return 2  +3; }" "5"
assert "int main() { return   2 +  3; }" "5"
assert "int main() { return 2+13; }" "15"
assert "int main() { return 3+14 +3 +4; }" "24"
assert "int main() { return 8-5; }" "3"
assert "int main() { return 8-5-1; }" "2"
assert "int main() { return 2+3-1; }" "4"
assert "int main() { return 4+6+1-2+12-5-2+5; }" "19"
assert "int main() { return 2*4; }" "8"
assert "int main() { return 1+2*4; }" "9"
assert "int main() { return 2*4+1; }" "9"
assert "int main() { return 4/2; }" "2"
assert "int main() { return 1+4/2; }" "3"
assert "int main() { return 4/2+1; }" "3"
assert "int main() { return 4/2+2*6/3; }" "6"
assert "int main() { 2; return 3; }" "3"
assert "int main() { 2; 3; return 5+4/2; }" "7"
assert "int main() { return 1; 2; 3; }" "1"
assert "int main() { 1; return 2; 3; }" "2"
assert "int main() { 1; 2; return 3; }" "3"
assert "int main() { int a=12; return a; }" "12"
assert "int main() { int a=12; a=13; return a; }" "13"
assert "int main() { int var=14; return var; }" "14"
assert "int main() { int a=12; int b=13; return a; }" "12"
assert "int main() { int a=12; int b=13; return b; }" "13"
assert "int main() { int var=14; int foo=15; int bar=16; return var; }" "14"
assert "int main() { int var=14; int foo=15; int bar=16; return foo; }" "15"
assert "int main() { int var=14; int foo=15; int bar=16; return bar; }" "16"
assert "int main() { int a=42; int var=14; int foo=15; int bar=16; return a; }" "42"
assert "int main() { int a=42; int var=14; int foo=15; int bar=16; return bar; }" "16"
assert "int main() { int a=42; a++; return a; }" "43"
assert "int main() { int a=42; a--; return a; }" "41"
assert "int main() { int a=1; int b=2; a=b=42; return a; }" "42"
assert "int main() { int a=1; int b=2; a=b=42; return b; }" "42"
assert "int main() { int a=1; int b=2; a=b=42; a++; return a; }" "43"
assert "int main() { int a=1; int b=2; a=b=42; a++; return b; }" "42"
assert "int ret42() { return 42; } int main() { return ret42(); }" "42"
assert "int ret42() { return 42; } int main() { int res = ret42(); return res; }" "42"
assert "int ret42() { return 42; } int main() { int res = ret42(); res++; return res; }" "43"
assert "int func() { int a=12; return 42; } int main() { int a=func(); return a; }" "42"
assert "int main() { int a=12; return sizeof(a); }" "8"
assert "int main() { int a=12; int b=sizeof(a); return b; }" "8"
assert "int main() { int a=23; int *p=&a; return *p; }" "23"
assert "int main() { int a=23; int *p=&a; int b=*p; return b; }" "23"
assert "int main() { int a=12; int b=23; int *p=&a; p=p+1; return *p; }" "23"
assert "int main() { int a=12; int b=23; int *p=&b; p=p-1; return *p; }" "12"
assert "int main() { int a=12; int b=23; int *p=&a; p++; return *p; }" "23"
assert "int main() { int a=12; int b=23; int *p=&b; p--; return *p; }" "12"
assert "int ret(int x) { return x; } int main() { int n=ret(12); return n; }" "12"
assert "int ret(int x) { return x; } int main() { return ret(23); }" "23"
assert "int add(int x, int y) { return x+y; } int main() { int n=add(1, 2); return n; }" "3"
assert "int add(int x, int y) { return x+y; } int main() { return add(1, 2); }" "3"
assert "int main() { if (1) return 12; return 23; }" "12"
assert "int main() { if (2) return 12; return 23; }" "12"
assert "int main() { if (0) return 12; return 23; }" "23"
assert "int main() { if (1) return 12; else return 23; return 34; }" "12"
assert "int main() { if (2) return 12; else return 23; return 34; }" "12"
assert "int main() { if (0) return 12; else return 23; return 34; }" "23"

# TODO: if-statement needed
# assert "int fib(int n) { if (n==0) return n; if (n==1) return n; return fib(n-2) + fib(n-1); } int main() { return fib(10); }" "55"

echo OK
exit 0
