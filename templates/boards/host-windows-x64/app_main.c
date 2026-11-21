#include <stdio.h>

#ifndef __x86_64__
#error "This application requires an x86-64 compiler."
#endif

int main(void)
{
    return puts("boot ok") < 0;
}
