#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_jvm.h"



void free_pools(JvmContext *ctx) 
{
    int i = 0;
    int j;
    MethodInfo *method =0;
    AttributeInfo *attr = 0;
    for ( i = 0 ; i < ctx->method_pool.method_used; i++ ) {
        method = &ctx->method_pool.method[i];
        for( j = 0 ; j < method->attributes_count ; j++ ) {
            attr = &method->attributes[j];
            free( attr->info );
            memset( attr, 0 , sizeof(AttributeInfo));
        }
       free(method->attributes);
       memset( method, 0 , sizeof(MethodInfo));
    }
}
