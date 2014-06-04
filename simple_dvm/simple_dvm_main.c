/*
 * Simple Dalvik Virtual Machine Project
 * This Project is created for elementary class
 * to help student to understand How it works.
 *
 * Auther : Chun-Yu Wang
 * Email  : wicanr2@gmail.com
 * Created Date : 2013/11/14
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_dvm.h"

int main(int argc, char* argv[]) {
    DexFileFormat dex;
    simple_dalvik_vm vm;
    int x = 0;

    memset( &dex, 0, sizeof(DexFileFormat));
#if 1
    if ( argc < 2 ) {
        printf("%s [dex_file] \n", argv[0]);
        return 0;
    }
    if ( argc >= 3 ) {
        set_verbose(atoi(argv[2]));
    }
    parseDexFile(argv[1], &dex); 
#else
    set_verbose(4);
    parseDexFile("Foo1.dex", &dex); 
#endif
    printDexFile(&dex);
    printf("-------------------------------------\n");
    printf("Execute Simple Dalvik Virtual Machine\n");
    printf("-------------------------------------\n");
    simple_dvm_startup(&dex, &vm,"main");
    printf("-------------------------------------\n");
    printf("Stop    Simple Dalvik Virtual Machine\n");
    printf("-------------------------------------\n");

    return 0;
}
