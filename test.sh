#!/bin/sh

echo "==========================================================="
echo "Input file: $1"
echo "start ====================================================="
(./af < $1) > "test_$2"

diff $2 "test_$2"

OUT=$?
if [ $OUT -eq 0 ];then
   echo "OK"
else
   echo "Fail. Exit status: $OUT"
fi

echo "end   ====================================================="
echo
