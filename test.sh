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

assert "return 7;" "7"
assert "return 42;" "42"
assert "return 123;" "123"
assert "return 2+3;" "5"
assert "return 2  +3;" "5"
assert "return   2 +  3;" "5"
assert "return 2+13;" "15"
assert "return 3+14 +3 +4;" "24"
assert "return 8-5;" "3"
assert "return 8-5-1;" "2"
assert "return 2+3-1;" "4"
assert "return 4+6+1-2+12-5-2+5;" "19"
assert "return 2*4;" "8"
assert "return 1+2*4;" "9"
assert "return 2*4+1;" "9"
assert "return 4/2;" "2"
assert "return 1+4/2;" "3"
assert "return 4/2+1;" "3"
assert "return 4/2+2*6/3;" "6"
assert "2; return 3;" "3"
assert "2; 3; return 5+4/2;" "7"
assert "return 1; 2; 3;" "1"
assert "1; return 2; 3;" "2"
assert "1; 2; return 3;" "3"

echo OK
exit 0
