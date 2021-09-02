# Case 1
./occ 42 > tmp.s
gcc -o tmp tmp.s

./tmp

ret="$?"
if [ "$ret" = "42" ]; then
  echo "42 => 42"
else
  echo "42 => $ret"
  exit 1
fi

# Case 2
./occ 123 > tmp.s
gcc -o tmp tmp.s

./tmp

ret="$?"
if [ "$ret" = "123" ]; then
  echo "123 => 123"
else
  echo "123 => $ret"
  exit 1
fi

echo OK
exit 0
