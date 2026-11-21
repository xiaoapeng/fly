#include <stdio.h>

#if !defined(__aarch64__) && !defined(__arm64__)
#error "This application requires an Arm64 compiler."
#endif

int main(void)
{
    return puts("boot ok") < 0;
}
