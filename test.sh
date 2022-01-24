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

assert "main() { return 42; }" "42"
assert "main() { return 123; }" "123"
assert "main() { return 42+3; }" "45"
assert "main() { return 42  + 13 ; }" "55"
assert "main() { return 42+3+12; }" "57"
assert "main() { return 42+3+12+2; }" "59"
assert "main() { return 44-2; }" "42"
assert "main() { return 44+22-34-12; }" "20"
assert "main() { return 2*34; }" "68"
assert "main() { return 2*3*4; }" "24"
assert "main() { return 2*34-3; }" "65"
assert "main() { return 12+2*4-3; }" "17"
assert "main() { return 12/4; }" "3"
assert "main() { return 12/5; }" "2"
assert "main() { return 12+4/2-3; }" "11"
assert "main() { return 12+3*4/2-3; }" "15"
assert "main() { return 1; 2; return 3; }" "1"
assert "main() { 1; return 2; return 3; }" "2"
assert "main() { 1; 2; return 3; }" "3"

echo OK
exit 0
