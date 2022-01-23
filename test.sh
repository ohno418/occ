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

assert "main() { 42; }" "42"
assert "main() { 123; }" "123"
assert "main() { 42+3; }" "45"
assert "main() {   42  + 13 ; }" "55"
assert "main() { 42+3+12; }" "57"
assert "main() { 42+3+12+2; }" "59"
assert "main() { 44-2; }" "42"
assert "main() { 44+22-34-12; }" "20"
assert "main() { 2*34; }" "68"
assert "main() { 2*3*4; }" "24"
assert "main() { 2*34-3; }" "65"
assert "main() { 12+2*4-3; }" "17"
assert "main() { 12/4; }" "3"
assert "main() { 12/5; }" "2"
assert "main() { 12+4/2-3; }" "11"
assert "main() { 12+3*4/2-3; }" "15"
assert "main() { 1; 2; 3; }" "3"

echo OK
exit 0
