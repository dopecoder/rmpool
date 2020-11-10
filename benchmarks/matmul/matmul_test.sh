#! /bin/bash
#
/usr/bin/matmul < matmul_input.txt > matmul_test.txt
if [ $? -ne 0 ]; then
  echo "Run error."
  exit
fi
#
echo "Normal end of execution."

