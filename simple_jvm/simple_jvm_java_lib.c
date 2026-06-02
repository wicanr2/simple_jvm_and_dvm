#include "simple_jvm_java_lib.h"
#include <time.h>

/*
 * 確定性測試用: 若設環境變數 SVM_SEED, 以該值為固定 seed; 否則用 time(0).
 * 只 seed 一次, 讓 golden 回歸測試在 SVM_SEED 下可重現。
 */
static void svm_seed_rand(void)
{
    static int seeded = 0;
    if (!seeded) {
        const char *s = getenv("SVM_SEED");
        srand(s != NULL ? (unsigned)atoi(s) : (unsigned)time(0));
        seeded = 1;
    }
}

int java_lang_math_random ( StackFrame *stack, SimpleConstantPool *p, char*type )
{
    double r = 0.0f;
    int i = 0 ;
    int times = 0;
    svm_seed_rand();
    times = rand()%100;
    for ( i = 0 ; i < times ; i++ ) { 
        r =((double)rand()/(double)RAND_MAX);
    }
#if SIMPLE_JVM_DEBUG
    printf("rand r = %f\n", r);
#endif
    pushDouble( stack, r);
    return 0;
}
static java_lang_method method_table[] = {
    {"java/lang/Math","random", java_lang_math_random}
};

static int java_lang_method_size = sizeof(method_table)/ sizeof(java_lang_method);

java_lang_method* find_java_lang_method( char *cls_name, char *method_name ) {
    int i = 0;
    for ( i = 0 ; i <java_lang_method_size; i++ ) {
        if ( strcmp(cls_name, method_table[i].clzname) == 0 && 
             strcmp(method_name, method_table[i].methodname) == 0 ) {
            return &method_table[i];
        }
    }  
    return 0;
}
int invoke_java_lang_library( 
        StackFrame *stack, SimpleConstantPool *p,
        char *cls_name, char *method_name, char *type ) 
{
    java_lang_method *method = find_java_lang_method(cls_name,method_name);
    if ( method != 0 ) {
#if SIMPLE_JVM_DEBUG
        printf("invoke %s/%s %s\n",method->clzname,method->methodname,type);
#endif
        method->method_runtime( stack, p, type );
        return 1;
    }
    return 0;
}
