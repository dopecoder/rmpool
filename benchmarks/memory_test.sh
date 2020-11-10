#! /bin/bash
#
gcc -c -Wall -I $HOME/include memory_test.c
if [ $? -ne 0 ]; then
  echo "Compile error."
  exit
fi
#
gcc memory_test.o -lm
if [ $? -ne 0 ]; then
  echo "Load error."
  exit
fi
#
rm memory_test.o
#
chmod ugo+x a.out
mv a.out /usr/bin/memory_test
#
echo "Normal end of execution."
