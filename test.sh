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

assert "42;" "42"
assert "123;" "123"
assert "42+3;" "45"
assert "  42  + 13 ;" "55"
assert "42+3+12;" "57"
assert "42+3+12+2;" "59"
assert "44-2;" "42"
assert "44+22-34-12;" "20"
assert "2*34;" "68"
assert "2*3*4;" "24"
assert "2*34-3;" "65"
assert "12+2*4-3;" "17"
assert "12/4;" "3"
assert "12/5;" "2"
assert "12+4/2-3;" "11"
assert "12+3*4/2-3;" "15"
assert "1; 2; 3;" "3"

echo OK
exit 0
