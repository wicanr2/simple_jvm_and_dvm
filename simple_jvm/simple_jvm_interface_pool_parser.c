#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_jvm.h"


// parse Interface Pool Class 
int parseIPClass( JvmContext *ctx, FILE *fp, int index ) {
    unsigned char short_tmp[2];
    ConstantClassRef *ptr = &ctx->interface_pool.clasz[ ctx->interface_pool.clasz_used ];

    ptr->tag = CONSTANT_CLASS;
    ptr->index = index;
    ptr->additional_byte_size = 2;

    fread( short_tmp, 2, 1, fp );
    ptr->stringIndex = short_tmp[0] << 8 | short_tmp[1];

    ctx->interface_pool.clasz_used++;
    return 0;
}

void printInterfacePool( SimpleConstantPool *p, SimpleInterfacePool *ip) {
    int i = 0;
    int j = 0;

    if ( ip->clasz_used > 0 ) {
        printf("Interface Class Pool= \n");
            for ( i = 0 ; i < ip->clasz_used ; i++ ) {
                ConstantUTF8 *ptr = findUTF8(p, ip->clasz[i].stringIndex );
                printf(" ip_index[%d], class[%d], tag = %d, size = %d, %d",
                        ip->clasz[i].index, i, ip->clasz[i].tag,
                        ip->clasz[i].additional_byte_size, 
                        ip->clasz[i].stringIndex
                      );
                if ( ptr != 0 ) {
                    printf(" ");
                    for ( j = 0 ; j < ptr->string_size ; j++ ) {
                        printf("%c",ptr->ptr[j]);
                    }
                    printf(" \n"); 
                } else {
                    printf("\n");
                }
            }
    }
}
int parseInterfacePool(JvmContext *ctx, FILE *fp, int count) {
    int i = 0; 
    for ( i = 1 ; i < count ; i ++ ) {
        parseIPClass(ctx, fp,1);
    }
    return 0;
}
