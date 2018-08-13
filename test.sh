#!/bin/bash

function compile {
  echo "$1" | ./self-c > tmp.s
  if [ $? -ne 0 ]; then
    echo "Failed to compile $1"
    exit
  fi
  gcc -o tmp.out test-driver.c tmp.s
  if [ $? -ne 0 ]; then
    echo "GCC failed"
    exit
  fi
}

function assertequal {
  if [ "$1" != "$2" ]; then
    echo "Test failed: $2 expected but got $1"
    exit
  fi
}

function test {
  compile "$2"
  assertequal "$(./tmp.out)" "$1"
}

make -s self-c

test 0 0

test 3 '1+2'
test 3 '1 + 2'
test 3 '1 + 1 + 1'
test 3 '1 - 1 + 3'
test 3 '5 - 2'
test 3 '8 - 4 - 1'
test 3 '3 * 1'
test 3 '9 / 3'
test 10 '1 + 3 * 3'
test 10 ' 3 * 3 + 1'
test 10 ' 3 * 1 + 6 / 2 + 4'

echo "All tests passed"
