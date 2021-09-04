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

assert "main() { return 7; }" "7"
assert "main() { return 42; }" "42"
assert "main() { return 123; }" "123"
assert "main() { return 2+3; }" "5"
assert "main() { return 2  +3; }" "5"
assert "main() { return   2 +  3; }" "5"
assert "main() { return 2+13; }" "15"
assert "main() { return 3+14 +3 +4; }" "24"
assert "main() { return 8-5; }" "3"
assert "main() { return 8-5-1; }" "2"
assert "main() { return 2+3-1; }" "4"
assert "main() { return 4+6+1-2+12-5-2+5; }" "19"
assert "main() { return 2*4; }" "8"
assert "main() { return 1+2*4; }" "9"
assert "main() { return 2*4+1; }" "9"
assert "main() { return 4/2; }" "2"
assert "main() { return 1+4/2; }" "3"
assert "main() { return 4/2+1; }" "3"
assert "main() { return 4/2+2*6/3; }" "6"
assert "main() { 2; return 3; }" "3"
assert "main() { 2; 3; return 5+4/2; }" "7"
assert "main() { return 1; 2; 3; }" "1"
assert "main() { 1; return 2; 3; }" "2"
assert "main() { 1; 2; return 3; }" "3"
assert "main() { a=12; return a; }" "12"
assert "main() { a=12; a=13; return a; }" "13"
assert "main() { var=14; return var; }" "14"
assert "main() { a=12; b=13; return a; }" "12"
assert "main() { a=12; b=13; return b; }" "13"
assert "main() { var=14; foo=15; bar=16; return var; }" "14"
assert "main() { var=14; foo=15; bar=16; return foo; }" "15"
assert "main() { var=14; foo=15; bar=16; return bar; }" "16"
assert "main() { a=42; var=14; foo=15; bar=16; return a; }" "42"
assert "main() { a=42; var=14; foo=15; bar=16; return bar; }" "16"
assert "main() { a=42; a++; return a; }" "43"
assert "main() { a=42; a--; return a; }" "41"

echo OK
exit 0
