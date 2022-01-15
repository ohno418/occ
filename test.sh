#!/bin/bash

./occ > tmp.s
gcc -o tmp tmp.s
./tmp
actual=$?

if [ $actual = 42 ]
then
  echo "OK"
  exit 0
else
  echo "Failed"
  exit 1
fi
