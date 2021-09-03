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

assert "7" "7"
assert "42" "42"
assert "123" "123"
assert "2+3" "5"
assert "2  +3" "5"
assert "  2 +  3 " "5"
assert "2+13" "15"
assert "3+14 +3 +4" "24"
assert "8-5" "3"
assert "8-5-1" "2"
assert "2+3-1" "4"
assert "4+6+1-2+12-5-2+5" "19"

echo OK
exit 0
