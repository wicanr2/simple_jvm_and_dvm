/*
 * Simple Java Virtual Machine Project
 * This Project is created for elementary class
 * to help student to understand How it works.
 *
 * Auther : Chun-Yu Wang
 * Email  : wicanr2@gmail.com
 * Created Date : 2013/11/05
 */
#ifndef SIMPLE_JVM_JAVA_LIBRARY_H
#define SIMPLE_JVM_JAVA_LIBRARY_H
#include "simple_jvm.h"
typedef int (*java_lang_lib) ( StackFrame *stack, SimpleConstantPool *p, char*type );

typedef struct _java_lang_method{
    char *clzname;
    char *methodname;
    java_lang_lib method_runtime;
} java_lang_method; 

int invoke_java_lang_library( 
        StackFrame *stack, SimpleConstantPool *p,
        char *cls_name, char *method_name, char *type ) ;
#endif
