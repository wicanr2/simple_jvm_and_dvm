#include <stdio.h>
void b_wrapper();
int main(int argc, char* argv[] ) {
    printf("call b_wrapper in a\n");
    b_wrapper();
}
