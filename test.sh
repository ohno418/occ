./occ > tmp.s
gcc -o tmp tmp.s

./tmp

if [ "$?" = "42" ]; then
  echo OK
  exit 0
else
  echo failed
  exit 1
fi
