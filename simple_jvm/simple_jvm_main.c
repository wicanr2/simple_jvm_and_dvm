/*
 * Simple Java Virtual Machine Project
 * This Project is created for elementary class
 * to help student to understand How it works.
 *
 * Auther : Chun-Yu Wang
 * Email  : wicanr2@gmail.com
 * Created Date : 2013/11/05
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_jvm.h"

/*
 * Simple JVM stores integer-only intrinsically,
 * and parses one class file called Foo.class in the same Folder.
 *
 * We limit its capability for quickly implementation.
 *
 * 所有執行狀態收進單一 JvmContext, 由 main 擁有、以指標顯式傳遞 (取代原本的全域 pool)。
 */
static JvmContext jvm;

int main(int argc, char* argv[]) {
    ClassFileFormat cff;

    /*
     * Initialize context and class file format
     */
    memset(&cff, 0, sizeof(ClassFileFormat));
    memset(&jvm, 0, sizeof(jvm));

    if ( argc < 2 ) {
        printf("%s [class] \n", argv[0]);
        return 0;
    }
    printf("open file %s\n", argv[1]);
    parseJavaClassFile(&jvm, argv[1], &cff);

#if SIMPLE_JVM_DEBUG
    printConstantPool( &jvm.constant_pool );
    printMethodPool( &jvm.constant_pool, &jvm.method_pool);
    printClassFileFormat(&cff);
#endif

    //TODO list method attributes
    printf("-------------------------------------\n");
    printf("Execute Simple JVM\n");
    printf("-------------------------------------\n");
    MethodInfo *init = findMethodInPool(
            &jvm.constant_pool,
            &jvm.method_pool,
            "<init>",6
            );
    if ( init != 0 ) {
        printf("-------------------------------------\n");
        printf("find and execute <init> method\n");
        printf("-------------------------------------\n");
#if SIMPLE_JVM_DEBUG
        printMethodAttributes(&jvm.constant_pool, init);
#endif
        stackInit( &jvm.stack, 500 );
        executeMethod( &jvm, init );
    }
    printf("-------------------------------------\n");
    printf("Terminate Simple JVM\n");
    printf("-------------------------------------\n");
    free_pools(&jvm);
    return 0;
}
