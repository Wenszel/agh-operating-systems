#include<stdio.h>
#include <dlfcn.h>

int main(void) {
    void *lib_handle = dlopen("./collatzbibl.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Nie udało się otworzyć biblioteki: %s\n", dlerror());
        return 1;
    }
    int (*test_collatz_conjecture)(int, int) = dlsym(lib_handle,"test_collatz_convergence"); 
    if (!test_collatz_conjecture) {
        fprintf(stderr, "Nie udało się załadować symbolu: %s\n", dlerror());
        dlclose(lib_handle);
        return 1;
    }

    int result = test_collatz_conjecture(10,100);
    printf("Dynamic: %d\n", result);

    dlclose(lib_handle);
    return 0;
}