#! /bin/bash
#
gcc -c -Wall matmul.c
if [ $? -ne 0 ]; then
  echo "Compile error."
  exit
fi
#
gcc matmul.o -lm
if [ $? -ne 0 ]; then
  echo "Load error."
  exit
fi
#
rm matmul.o
#
chmod ugo+x a.out
mv a.out /usr/bin/matmul
#
echo "Normal end of execution."
