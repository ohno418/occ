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
# assert "2+3" "5"
# assert "2  +3" "5"
# assert "  2 +  3 " "5"
# assert "2+13" "15"

echo OK
exit 0
