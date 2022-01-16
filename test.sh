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

assert "42" "42"
assert "123" "123"
assert "42+3" "45"
assert "  42  + 13 " "55"
assert "42+3+12" "57"
assert "42+3+12+2" "59"

echo OK
exit 0
