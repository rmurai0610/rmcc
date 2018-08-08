#include <stdio.h>

extern int mymain(void);

int main(int argc, char const* argv[]) {
    int val = mymain();
    printf("%d\n", val);
    return 0;
}
