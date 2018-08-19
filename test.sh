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

test 0  'int main() { return 0; }'
test 3  'int main() { return 1+2; }'
test 3  'int main() { return 1 + 2; }'
test 3  'int main() { return 1 + 1 + 1; }'
test 3  'int main() { return 1 - 1 + 3; }'
test 3  'int main() { return 5 - 2; }'
test 3  'int main() { return 8 - 4 - 1; }'
test 3  'int main() { return 3 * 1; }'
test 3  'int main() { return 9 / 3; }'
test 10 'int main() { return 1 + 3 * 3; }'
test 10 'int main() { return 3 * 3 + 1; }'
test 10 'int main() { return 3 * 1 + 6 / 2 + 4; }'
test 10 'int main() { return 6 / 1 / 6 + 9; }'
test 10 'int main() { int a = 9; int b = 1; return a + b; }'
test 10 'int a() { return 10; } int main() { return a(); }'
test 10 'int a() { int a = 1; return 9 + a; } int main() { return a(); }'
test 10 'int a() { int a = 1; return 5 + a; } int main() { return 4 + a();  }'
test 10 'int a() { int a = 1; return 9 + a; } int main() { return 1 * a();  }'
test 10 'int a(int a) { return a; } int main() { return 1 + a(9);  }'

echo "All tests passed"
