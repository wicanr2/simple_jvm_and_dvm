#include <cstdio>

extern "C" {
    void b_wrapper();
}

namespace B{
    void b_func();
}

void b_wrapper() {
    printf("call b wrapper\n");
    B::b_func();
}
