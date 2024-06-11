#include<stdio.h>
#include "collatzbibl.h"
#include <dlfcn.h>

int main(void) {
printf("Static: %d \n", test_collatz_convergence(15, 1));
    return 0;
}