#include "simple_dvm.h"
#include "simple_dvm_java_lib.h"
int find_const_string(DexFileFormat *dex, char *entry) {
    unsigned int i = 0;
    for ( i = 0 ; i < dex->header.stringIdsSize ; i++ ) {
        if ( memcmp(dex->string_data_item[i].data, entry, strlen(entry)) == 0 )
        {
            if ( is_verbose() ) {
                printf("find %s in dex->string_data_item[%d]\n", entry, i );
            }
            return i;
        }
    }
    return -1;
}
void printRegs(  simple_dalvik_vm *vm ) {
    int i = 0; 
    if ( is_verbose() ) {
        printf("pc = %08x\n", vm->pc);
        for ( i = 0 ; i < 16 ; i ++ ) {
            printf("Reg[%2d] = %4d (%04x) ",
                    i, *(int *)vm->regs[i].data, *(unsigned int *)vm->regs[i].data);
            if ( (i+1)%4 == 0 ) printf("\n");
        }
    }
}
/*
   0x0b , move-result-wide

   Move the long/double result value of the 
   previous method invocation into vx,vx+1.

   0B02 - move-result-wide v2
   Move the long/double result value of the previous method 
   invocation into v2,v3.
*/
int op_move_result_wide
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    reg_idx_vx = ptr[*pc+1]  ;
    reg_idx_vy = reg_idx_vx + 1;
    if ( is_verbose() ) {
        printf("move-result-wide v%d,v%d\n", reg_idx_vx,reg_idx_vy );
    }
    move_bottom_half_result_to_reg (vm, reg_idx_vx); 
    move_top_half_result_to_reg(vm, reg_idx_vy); 
    *pc = *pc + 2;
    return 0;
}
/*
   0x0c ,move-result-object vx

   Move the result object reference of 
   the previous method invocation into vx.

   0C00 - move-result-object v0
*/
int op_move_result_object
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    reg_idx_vx = ptr[*pc+1]  ;
    if ( is_verbose() ) {
        printf("move-result-object v%d\n", reg_idx_vx );
    }
    move_bottom_half_result_to_reg (vm, reg_idx_vx); 
    *pc = *pc + 2;
    return 0;
}

//0x0e , return-void
//Return without a return value 
//0E00 - return-void
int op_return_void
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    if ( is_verbose() ) {
        printf("return-void\n");
    }
    *pc = *pc + 2;
    return 0;
}

//0x12, const/4 vx,lit4
//Puts the 4 bit constant into vx
//1221 - const/4 v1, #int2
//Moves literal 2 into v1. 
//The destination register is in the lower 4 bit 
//in the second byte, the literal 2 is in the higher 4 bit.
int op_const_4( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int value = 0;
    int reg_idx_vx = 0;
    value = ptr[*pc+1] >> 4 ;
    reg_idx_vx = ptr[*pc+1] & 0x0F ;
    if ( value & 0x08 ) {
        value = 0x0F - value + 1;
        value = -value ;
    }
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&value);
    if ( is_verbose() ) {
        printf("const/4 v%d, #int%d\n", reg_idx_vx , value);
    }
    *pc = *pc + 2;
    return 0;
}

//0x13, const/16 vx,lit16
//Puts the 16 bit constant into vx
//1300 0A00 - const/16 v0, #int 10
//Puts the literal constant of 10 into v0.
int op_const_16( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int value = 0;
    reg_idx_vx= ptr[*pc+1] ;
    value = ( ptr[*pc+3] << 8 | ptr[*pc+2] );

    store_to_reg(vm,reg_idx_vx, (unsigned char*)&value);
    if ( is_verbose() ) {
        printf("const/16 v%d, #int%d\n", reg_idx_vx, value);
    }
    *pc = *pc + 4;
    return 0;
}
/*
//0x19, const-wide/high16 vx,lit16
//Puts the 16 bit constant into the highest 16 bit of vx 
//and vx+1 registers. 
//Used to initialize double values.
//1900 2440 - const-wide/high16 v0, #double 10.0 // #402400000
//Puts the double constant of 10.0 into v0 register.
*/
int op_const_wide_high16( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    long long value = 0L;
    unsigned char *ptr2 = (unsigned char*) &value;
    int reg_idx_vx = 0;
    reg_idx_vx = ptr[*pc+1] ;
    ptr2[1] = ptr[*pc+3];
    ptr2[0] = ptr[*pc+2];
    if ( is_verbose() ) {
        printf("const-wide/hight16 v%d, #long %lld\n", reg_idx_vx, value);
    }
    store_to_reg ( vm, reg_idx_vx, ptr2 );
    value = 1L;
    *pc = *pc + 4;
    return 0;
}
//0x1a, const-string vx,string_id 
//Puts reference to a string constant identified by string_id into vx.
//1A08 0000 - const-string v8, "" // string@0000
//Puts reference to string@0000 (entry #0 in the string table) into v8.
int op_const_string ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int string_id = 0;
    reg_idx_vx = ptr[*pc+1];
    string_id = ((ptr[*pc+3] << 8) | ptr[*pc+2]);

    if ( is_verbose() ) {
        printf("const-string v%d, string_id 0x%04x\n", 
                reg_idx_vx , string_id );
    }
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&string_id);
    *pc = *pc + 4;
    return 0;

}
// 0x22 new-instance vx,type
// Instantiates an object type and puts 
// the reference of the newly created instance into vx
// 2200 1500 - new-instance v0, java.io.FileInputStream // type@0015
// Instantiates type@0015 (entry #15H in the type table) 
// and puts its reference into v0.
int op_new_instance ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int type_id = 0;
    type_id_item *type_item = 0;

    reg_idx_vx = ptr[*pc+1];
    type_id = ((ptr[*pc+3] << 8) | ptr[*pc+2]);

    type_item = get_type_item( dex, type_id );
    
    if ( is_verbose() ) {
        printf("new-instance v%d, type_id 0x%04x", reg_idx_vx , type_id );
        if ( type_item != 0 ) {
            printf(" %s",get_string_data(dex, type_item->descriptor_idx)); 
        }
        printf("\n");
    }
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&type_id);
    //TODO
    *pc = *pc + 4;
    return 0;

}
/*
 * 35c format
 * A|G|op BBBB F|E|D|C
 * [A=5] op {vC, vD, vE, vF, vG}, meth@BBBB
 * [A=5] op {vC, vD, vE, vF, vG}, type@BBBB
 * [A=4] op {vC, vD, vE, vF}, kind@BBBB
 * [A=3] op {vC, vD, vE}, kind@BBBB
 * [A=2] op {vC, vD}, kind@BBBB
 * [A=1] op {vC}, kind@BBBB
 * [A=0] op {}, kind@BBBB
 * The unusual choice in lettering here reflects a desire to 
 * make the count and the reference index have the same label as in format 3rc.
 * */
int op_utils_invoke_35c_parse( 
        DexFileFormat *dex, u1 *ptr, int *pc,
        invoke_parameters *p)
{
    unsigned char tmp = 0;
    if ( dex != 0 && ptr != 0 && p != 0 ) {
        memset(p, 0, sizeof(invoke_parameters));

        tmp = ptr[*pc+1];
        p->reg_count = tmp >> 4;
        p->reg_idx[4] = tmp & 0x0F;

        p->method_id = ptr[*pc+2];
        p->method_id |= (ptr[*pc+3] << 4);

        tmp = ptr[*pc+4];
        p->reg_idx[1] = tmp >> 4;
        p->reg_idx[0] = tmp & 0x0F;

        tmp = ptr[*pc+5];
        p->reg_idx[3] = tmp >> 4;
        p->reg_idx[2] = tmp & 0x0F;
#if 0
        printf("parse regiser:\n");
        for ( i = 0 ; i < 5 ; i++ ) {
            printf("reg[%d] = %d\n", i, p->reg_idx[i]);
        }
#endif
    }
    return 0;
}

int op_utils_invoke( char *name, DexFileFormat *dex, simple_dalvik_vm *vm, invoke_parameters *p)
{
    method_id_item *m = 0;
    type_id_item *type_class = 0;
    proto_id_item *proto_item = 0;
    type_list *proto_type_list = 0;
    if ( p != 0 ) {
        m = get_method_item(dex, p->method_id );
        if ( m != 0 ) {
            type_class = get_type_item( dex, m->class_idx );
            proto_item = get_proto_item( dex, m->proto_idx );
        }
        switch(p->reg_count) {
            case 0:

                if ( is_verbose() ) {
                    printf("%s {} method_id 0x%04x", name, p->method_id );
                }
                break;
            case 1:
                if ( is_verbose() ) {
                    printf("%s, {v%d} method_id 0x%04x", 
                            name, p->reg_idx[0], p->method_id );
                }
                break;
            case 2:
                if ( is_verbose() ) {
                    printf("%s {v%d, v%d} method_id 0x%04x", 
                            name, 
                            p->reg_idx[0], p->reg_idx[1], 
                            p->method_id );
                }
                break;
            case 3:
                if ( is_verbose() ) {
                    printf("%s {v%d, v%d, v%d} method_id 0x%04x", 
                            name, 
                            p->reg_idx[0], p->reg_idx[1], p->reg_idx[2],
                            p->method_id );
                }
                break;
            case 4:
                if ( is_verbose() ) {
                    printf("%s {v%d, v%d, v%d, v%d} method_id 0x%04x", 
                            name, 
                            p->reg_idx[0], p->reg_idx[1], 
                            p->reg_idx[2], p->reg_idx[3],
                            p->method_id );
                }
                break;
            case 5:
                if ( is_verbose() ) {
                    printf("%s {v%d, v%d, v%d, v%d, v%d} method_id 0x%04x", 
                            name, 
                            p->reg_idx[0], p->reg_idx[1], p->reg_idx[2], 
                            p->reg_idx[3], p->reg_idx[4],
                            p->method_id );
                }
                break;
            default:
                break;
        }

        if ( m!=0 && type_class != 0 && p->reg_count <= 5) 
        {
            if ( proto_item != 0 ){
                proto_type_list = get_proto_type_list(dex, m->proto_idx ); 
            }
            if ( proto_type_list != 0 && proto_type_list->size > 0 ) {
                if ( is_verbose() ) {
                    printf(" %s,%s,(%s)%s \n", 
                            get_string_data(dex, type_class->descriptor_idx), 
                            get_string_data(dex, m->name_idx),
                            get_type_item_name( dex, 
                                proto_type_list->type_item[0].type_idx),
                            get_type_item_name( dex, 
                                proto_item->return_type_idx)
                          );
                }
                invoke_java_lang_library(dex,vm,
                    get_string_data(dex, type_class->descriptor_idx),
                    get_string_data(dex, m->name_idx),
                     get_type_item_name( dex, proto_type_list->type_item[0].type_idx));
            }else{
                if ( is_verbose() ) {
                    printf(" %s,%s,()%s \n", 
                            get_string_data(dex, type_class->descriptor_idx), 
                            get_string_data(dex, m->name_idx),
                            get_type_item_name( dex, 
                                proto_item->return_type_idx)
                          );
                }
                invoke_java_lang_library(dex,vm,
                    get_string_data(dex, type_class->descriptor_idx),
                    get_string_data(dex, m->name_idx), 0);
            }
            
        } else {
            if ( is_verbose() ) { printf("\n"); }
        }
    }
    return 0;
}


// invoke-virtual { parameters }, methodtocall
/*
 * 6E53 0600 0421 - invoke-virtual { v4, v0, v1, v2, v3}, Test2.method5:(IIII)V // method@0006
 * 6e20 0200 3200   invoke-virtual {v2, v3}, Ljava/io/PrintStream;.println:(Ljava/lang/String;)V // method@0002
 * */
int op_invoke_virtual ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    op_utils_invoke_35c_parse(dex, ptr, pc, &vm->p);
    op_utils_invoke("invoke-virtual", dex, vm, &vm->p);
    //TODO
    *pc = *pc + 6;
    return 0;

}
// invoke-direct
// 7010 0400 0300  invoke-direct {v3}, Ljava/lang/StringBuilder;.<init>:()V // method@0004
int op_invoke_direct ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    op_utils_invoke_35c_parse(dex, ptr, pc, &vm->p);
    op_utils_invoke("invoke-direct", dex, vm, &vm->p);
    //TODO
    *pc = *pc + 6;
    return 0;

}
// 0x71 invoke-direct
// 7100 0300 0000  invoke-static {}, Ljava/lang/Math;.random:()D // method@0003
int op_invoke_static ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    op_utils_invoke_35c_parse(dex, ptr, pc, &vm->p);
    op_utils_invoke("invoke-static", dex, vm, &vm->p);
    //TODO
    *pc = *pc + 6;
    return 0;

}
// 0x62 sget-object vx,field_id
// Reads the object reference field identified by the field_id into vx.
// 6201 0C00 - sget-object v1, Test3.os1:Ljava/lang/Object; // field@000c
// Reads field@000c (entry #CH in the field id table) into v1.
int op_sget_object ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int field_id = 0;
    int reg_idx_vx = 0;
    reg_idx_vx = ptr[*pc+1];
    field_id = ((ptr[*pc+3] << 8) | ptr[*pc+2]);

    if ( is_verbose() ) {
        printf("sget-object v%d, field 0x%04x\n", reg_idx_vx , field_id);
    }
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&field_id);
    //TODO
    *pc = *pc + 4;
    return 0;

}
// 0x90 add-int vx,vy vz
// Calculates vy+vz and puts the result into vx.
// 9000 0203 - add-int v0, v2, v3 
// Adds v3 to v2 and puts the result into v0.
int op_add_int( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int reg_idx_vz = 0;
    int x = 0, y =0 , z =0;
    reg_idx_vx = ptr[*pc+1];
    reg_idx_vy = ptr[*pc+2];
    reg_idx_vz = ptr[*pc+3];

    if ( is_verbose() ) {
        printf("add-int v%d, v%d, v%d\n", reg_idx_vx, reg_idx_vy, 
                reg_idx_vz);
    }
    // x = y + z
    load_reg_to( vm,reg_idx_vy, (unsigned char*)&y);
    load_reg_to( vm,reg_idx_vz, (unsigned char*)&z);
    x = y + z;
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&x);
    *pc = *pc + 4;
    return 0;

}
//0x91 sub-int vx,vy,vz
//Calculates vy-vz and puts the result into vx.
//9100 0203 - sub-int v0, v2, v3
//Subtracts v3 from v2 and puts the result into v0.
int op_sub_int( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int reg_idx_vz = 0;
    int x = 0, y =0 , z =0;
    reg_idx_vx = ptr[*pc+1];
    reg_idx_vy = ptr[*pc+2];
    reg_idx_vz = ptr[*pc+3];

    if ( is_verbose() ) {
        printf("sub-int v%d, v%d, v%d\n", reg_idx_vx, reg_idx_vz, 
                reg_idx_vy);
    }
    // x = y + z
    load_reg_to( vm,reg_idx_vy, (unsigned char*)&y);
    load_reg_to( vm,reg_idx_vz, (unsigned char*)&z);
    x = y - z;
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&x);
    *pc = *pc + 4;
    return 0;

}
//0x92 mul-int vx, vy, vz
//Multiplies vz with wy and puts the result int vx.
//9200 0203 - mul-int v0,v2,v3
//Multiplies v2 with w3 and puts the result into v0
int op_mul_int( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int reg_idx_vz = 0;
    int x = 0, y =0 , z =0;
    reg_idx_vx = ptr[*pc+1];
    reg_idx_vy = ptr[*pc+2];
    reg_idx_vz = ptr[*pc+3];

    if ( is_verbose() ) {
        printf("add-int v%d, v%d, v%d\n", reg_idx_vx, reg_idx_vy, reg_idx_vz);
    }
    // x = y + z
    load_reg_to( vm,reg_idx_vy, (unsigned char*)&y);
    load_reg_to( vm,reg_idx_vz, (unsigned char*)&z);
    x = y * z;
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&x);
    *pc = *pc + 4;
    return 0;

}
//0x93 div-int vx,vy,vz
//Divides vy with vz and puts the result into vx.
//9303 0001 - div-int v3, v0, v1
//Divides v0 with v1 and puts the result into v3.
int op_div_int( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int reg_idx_vz = 0;
    int x = 0, y =0 , z =0;
    reg_idx_vx = ptr[*pc+1];
    reg_idx_vy = ptr[*pc+2];
    reg_idx_vz = ptr[*pc+3];

    if ( is_verbose() ) {
        printf("add-int v%d, v%d, v%d\n", reg_idx_vx, reg_idx_vy, reg_idx_vz);
    }
    // x = y + z
    load_reg_to( vm,reg_idx_vy, (unsigned char*)&y);
    load_reg_to( vm,reg_idx_vz, (unsigned char*)&z);
    x = y % z;
    x = (y-x) / z;
    store_to_reg(vm,reg_idx_vx, (unsigned char*)&x);
    *pc = *pc + 4;
    return 0;

}

//0x8A double-to-int vx, vy
//Converts the double value in vy,vy+1 into an integer value in vx.
//8A40  - double-to-int v0, v4
//Converts the double value in v4,v5 into an integer value in v0.
int op_double_to_int
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int reg_idx_vz = 0;
    double d=0;
    unsigned char *ptr_d = (unsigned char*)&d;
    int i=0;
    reg_idx_vx = ptr[*pc+1] & 0x0F ;
    reg_idx_vy = (ptr[*pc+1] >> 4) & 0x0F ;
    reg_idx_vz = reg_idx_vy +1  ;
   
    load_reg_to_double( vm, reg_idx_vy , ptr_d+4 ); 
    load_reg_to_double( vm, reg_idx_vz , ptr_d ); 

    i = (int)d;
    if ( is_verbose() ) {
        printf("double-to-int v%d, v%d\n", reg_idx_vx, reg_idx_vy );
        printf("(%f) to (%d) \n", d , i);
    }
    
    store_to_reg( vm, reg_idx_vx, (unsigned char*)&i); 
    *pc = *pc + 2;
    return 0;
}
//0xb0 add-int/2addr vx,vy 
//Adds vy to vx.
//B010 - add-int/2addr v0,v1 Adds v1 to v0.
int op_add_int_2addr
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int x = 0, y = 0;
    reg_idx_vx = ptr[*pc+1] & 0x0F ;
    reg_idx_vy = (ptr[*pc+1] >> 4) & 0x0F ;
    if ( is_verbose() ) {
        printf("add-int/2addr v%d, v%d\n", reg_idx_vx, reg_idx_vy );
    }
    load_reg_to( vm,reg_idx_vx, (unsigned char*)&x);
    load_reg_to( vm,reg_idx_vy, (unsigned char*)&y);
    x = x + y;
    store_to_reg( vm, reg_idx_vx, (unsigned char*)&x); 
   
    *pc = *pc + 2;
    return 0;
}
//0xcb , add-double/2addr 
//Adds vy to vx.
//CB70 - add-double/2addr v0, v7
//Adds v7 to v0.
int op_add_double_2addr
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    double x = 0.0, y = 0.0;
    unsigned char *ptr_x = (unsigned char*)&x;
    unsigned char *ptr_y = (unsigned char*)&y;
    reg_idx_vx = ptr[*pc+1] & 0x0F ;
    reg_idx_vy = (ptr[*pc+1] >> 4) & 0x0F ;
    
    load_reg_to_double(vm, reg_idx_vx, ptr_x+4);
    load_reg_to_double(vm, reg_idx_vx+1, ptr_x);
    load_reg_to_double(vm, reg_idx_vy, ptr_y+4);
    load_reg_to_double(vm, reg_idx_vy+1, ptr_y);
    

    if ( is_verbose() ) {
        printf("add-double/2addr v%d, v%d\n", reg_idx_vx, reg_idx_vy );
        printf("%f(%llx) + %f(%llx) = %f\n",
               x, *(unsigned long long *)&x, y, *(unsigned long long *)&y, y+x );
    }
    x = x + y;
    store_double_to_reg(vm, reg_idx_vx, ptr_x+4);
    store_double_to_reg(vm, reg_idx_vx+1, ptr_x);
    *pc = *pc + 2;
    return 0;
}
//0xcd , mul-double/2addr 
//Multiplies vx with vy
//CD20 - mul-double/2addr v0, v2
//Multiplies the double value in v0,v1 with the 
//double value in v2,v3 and puts the result into v0,v1.
int op_mul_double_2addr
    ( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int reg_idx_vz = 0;
    int reg_idx_vw = 0;
    double x = 0.0, y = 0.0;
    unsigned char *ptr_x = (unsigned char*)&x;
    unsigned char *ptr_y = (unsigned char*)&y;

    reg_idx_vx = ptr[*pc+1] & 0x0F ;
    reg_idx_vy = reg_idx_vx + 1 ;
    reg_idx_vz = (ptr[*pc+1] >> 4) & 0x0F ;
    reg_idx_vw = reg_idx_vz + 1 ;

    load_reg_to_double(vm, reg_idx_vx, ptr_x+4);
    load_reg_to_double(vm, reg_idx_vy, ptr_x);
#if 0 
    x = 18.0f;
    printf("x = %f, %llx\n", x, x);
#endif

    load_reg_to_double(vm, reg_idx_vz, ptr_y+4);
    load_reg_to_double(vm, reg_idx_vw, ptr_y);

#if 0 
    printf("y = %f, %llx\n", y, y);
#endif

    if ( is_verbose() ) {
        printf("mul-double/2addr v%d, v%d\n", reg_idx_vx, reg_idx_vz );
        printf(" %f * %f = %f\n", x, y, x*y);
    }

    x = x * y;

#if 0 
    printf("x = %f, %llx\n", x, x);
#endif

    store_double_to_reg(vm, reg_idx_vx, ptr_x+4); 
    store_double_to_reg(vm, reg_idx_vy, ptr_x); 

    load_reg_to_double(vm, reg_idx_vx, ptr_y+4);
    load_reg_to_double(vm, reg_idx_vy, ptr_y);

#if 0 
    printf("y = %f, %llx\n", y, y);
    printf("inverse convert y = %f \n", y);
#endif
    *pc = *pc + 2;
    return 0;
}

//0xdb div-int/lit8 vx,vy,lit8
//Calculates vy/lit8 and stores the result into vx.
//DB00 0203 - div-int/lit8 v0,v2, #int3
//Calculates v2/3 and stores the result into v0.
int op_div_int_lit8( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc ) 
{
    int reg_idx_vx = 0;
    int reg_idx_vy = 0;
    int x = 0, y =0 ;
    int z = 0;
    reg_idx_vx = ptr[*pc+1];
    reg_idx_vy = ptr[*pc+2];
    z = ptr[*pc+3];

    if ( is_verbose() ) {
        printf("add-int v%d, v%d, #int%d\n", reg_idx_vx, reg_idx_vy, z);
    }
    // x = y + z
    load_reg_to ( vm, reg_idx_vy, (unsigned char*)&y);
    x = y % z;
    x = (y - x)/z;
    store_to_reg(vm, reg_idx_vx, (unsigned char*)&x);

    *pc = *pc + 4;
    return 0;

}
//==================================================================
// 陣列 / 迴圈 opcode (GEMM 範例所需)
//==================================================================
// 0x23 new-array vA, vB, type@CCCC : vA = new int[vB]  (format 22c)
int op_new_array( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    int type_id = (ptr[*pc+3] << 8) | ptr[*pc+2];
    int count = 0;
    int handle = vm->array_count;
    int n = 0;
    char *tname = get_type_item_name(dex, type_id);   // "[I" / "[D"
    int is_double = ( tname != 0 && tname[0] == '[' && tname[1] == 'D' );
    load_reg_to(vm, vB, (unsigned char*)&count);
    n = count > 0 ? count : 1;
    if ( handle < DVM_MAX_ARRAYS ) {
        vm->array_count++;
        vm->arrays[handle].length = count;
        vm->arrays[handle].is_double = is_double;
        if ( is_double )
            vm->arrays[handle].ddata = (double*) calloc(n, sizeof(double));
        else
            vm->arrays[handle].data = (int*) calloc(n, sizeof(int));
    }
    store_to_reg(vm, vA, (unsigned char*)&handle);
    if ( is_verbose() )
        printf("new-array v%d, v%d (%s[%d]) -> handle %d\n", vA, vB, is_double?"double":"int", count, handle);
    *pc = *pc + 4;
    return 0;
}
//------------------------------------------------------------------
// double (wide) opcode (浮點 GEMM 所需); 沿用 add-double/2addr 的 reg pair 慣例
//------------------------------------------------------------------
// 0x18 const-wide vAA, #+lit64 : 載入 64-bit double 字面值到 reg pair  (51l, 10 bytes)
int op_const_wide( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vAA = ptr[*pc+1];
    double d = 0.0;
    unsigned char *p = (unsigned char*)&d;
    memcpy(&d, &ptr[*pc+2], 8);               // little-endian IEEE754 bits
    store_double_to_reg(vm, vAA,   p+4);
    store_double_to_reg(vm, vAA+1, p);
    if ( is_verbose() ) printf("const-wide v%d, %f\n", vAA, d);
    *pc = *pc + 10;
    return 0;
}
// 0x16 const-wide/16 vAA, #+int16 : 16-bit 號誌延伸成 64-bit (GEMM 只用於 0.0)  (21s, 4 bytes)
int op_const_wide_16( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vAA = ptr[*pc+1];
    short s = (short)((ptr[*pc+3] << 8) | ptr[*pc+2]);
    long long ll = (long long) s;             // 對 0 而言 long 0 == double 0.0 (bits 相同)
    unsigned char *p = (unsigned char*)&ll;
    store_double_to_reg(vm, vAA,   p+4);
    store_double_to_reg(vm, vAA+1, p);
    if ( is_verbose() ) printf("const-wide/16 v%d, %d\n", vAA, (int)s);
    *pc = *pc + 4;
    return 0;
}
// 0x45 aget-wide vAA, vBB, vCC : vAA,vAA+1 = darray(vBB)[vCC]  (23x)
int op_aget_wide( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vAA = ptr[*pc+1];
    int vBB = ptr[*pc+2];
    int vCC = ptr[*pc+3];
    int handle = 0, index = 0;
    double d = 0.0;
    unsigned char *p = (unsigned char*)&d;
    load_reg_to(vm, vBB, (unsigned char*)&handle);
    load_reg_to(vm, vCC, (unsigned char*)&index);
    if ( handle >= 0 && handle < vm->array_count && vm->arrays[handle].is_double &&
         vm->arrays[handle].ddata && index >= 0 && index < vm->arrays[handle].length )
        d = vm->arrays[handle].ddata[index];
    store_double_to_reg(vm, vAA,   p+4);
    store_double_to_reg(vm, vAA+1, p);
    if ( is_verbose() ) printf("aget-wide v%d, v%d[v%d] = %f\n", vAA, vBB, vCC, d);
    *pc = *pc + 4;
    return 0;
}
// 0x4c aput-wide vAA, vBB, vCC : darray(vBB)[vCC] = vAA,vAA+1  (23x)
int op_aput_wide( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vAA = ptr[*pc+1];
    int vBB = ptr[*pc+2];
    int vCC = ptr[*pc+3];
    int handle = 0, index = 0;
    double d = 0.0;
    unsigned char *p = (unsigned char*)&d;
    load_reg_to_double(vm, vAA,   p+4);
    load_reg_to_double(vm, vAA+1, p);
    load_reg_to(vm, vBB, (unsigned char*)&handle);
    load_reg_to(vm, vCC, (unsigned char*)&index);
    if ( handle >= 0 && handle < vm->array_count && vm->arrays[handle].is_double &&
         vm->arrays[handle].ddata && index >= 0 && index < vm->arrays[handle].length )
        vm->arrays[handle].ddata[index] = d;
    if ( is_verbose() ) printf("aput-wide v%d -> v%d[v%d] = %f\n", vAA, vBB, vCC, d);
    *pc = *pc + 4;
    return 0;
}
// 0x44 aget vA, vB, vC : vA = array(vB)[vC]  (format 23x)
int op_aget( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1];
    int vB = ptr[*pc+2];
    int vC = ptr[*pc+3];
    int handle = 0, index = 0, value = 0;
    load_reg_to(vm, vB, (unsigned char*)&handle);
    load_reg_to(vm, vC, (unsigned char*)&index);
    if ( handle >= 0 && handle < vm->array_count &&
         index >= 0 && index < vm->arrays[handle].length )
        value = vm->arrays[handle].data[index];
    store_to_reg(vm, vA, (unsigned char*)&value);
    if ( is_verbose() )
        printf("aget v%d, v%d[v%d] = %d\n", vA, vB, vC, value);
    *pc = *pc + 4;
    return 0;
}
// 0x4b aput vA, vB, vC : array(vB)[vC] = vA  (format 23x)
int op_aput( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1];
    int vB = ptr[*pc+2];
    int vC = ptr[*pc+3];
    int handle = 0, index = 0, value = 0;
    load_reg_to(vm, vA, (unsigned char*)&value);
    load_reg_to(vm, vB, (unsigned char*)&handle);
    load_reg_to(vm, vC, (unsigned char*)&index);
    if ( handle >= 0 && handle < vm->array_count &&
         index >= 0 && index < vm->arrays[handle].length )
        vm->arrays[handle].data[index] = value;
    if ( is_verbose() )
        printf("aput v%d -> v%d[v%d] = %d\n", vA, vB, vC, value);
    *pc = *pc + 4;
    return 0;
}
// 0x01 move vA, vB : vA = vB  (format 12x)
int op_move( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    int value = 0;
    load_reg_to(vm, vB, (unsigned char*)&value);
    store_to_reg(vm, vA, (unsigned char*)&value);
    if ( is_verbose() ) printf("move v%d, v%d (%d)\n", vA, vB, value);
    *pc = *pc + 2;
    return 0;
}
// 0xb2 mul-int/2addr vA, vB : vA = vA * vB  (format 12x)
int op_mul_int_2addr( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    int x = 0, y = 0;
    load_reg_to(vm, vA, (unsigned char*)&x);
    load_reg_to(vm, vB, (unsigned char*)&y);
    x = x * y;
    store_to_reg(vm, vA, (unsigned char*)&x);
    if ( is_verbose() ) printf("mul-int/2addr v%d, v%d = %d\n", vA, vB, x);
    *pc = *pc + 2;
    return 0;
}
// 0xd8 add-int/lit8 vA, vB, #C : vA = vB + C  (format 22b)
int op_add_int_lit8( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1];
    int vB = ptr[*pc+2];
    signed char lit = (signed char) ptr[*pc+3];
    int y = 0;
    load_reg_to(vm, vB, (unsigned char*)&y);
    y = y + lit;
    store_to_reg(vm, vA, (unsigned char*)&y);
    if ( is_verbose() ) printf("add-int/lit8 v%d, v%d, #%d = %d\n", vA, vB, (int)lit, y);
    *pc = *pc + 4;
    return 0;
}
// 0x28 goto +AA : 8-bit signed offset (以 16-bit code unit 計)  (format 10t)
int op_goto( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    signed char off = (signed char) ptr[*pc+1];
    if ( is_verbose() ) printf("goto %+d (units)\n", (int)off);
    *pc = *pc + off * 2;
    return 0;
}
// 0x35 if-ge vA, vB, +CCCC : vA>=vB 則跳 (16-bit signed, code unit)  (format 22t)
int op_if_ge( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    short off = (short)((ptr[*pc+3] << 8) | ptr[*pc+2]);
    int x = 0, y = 0;
    load_reg_to(vm, vA, (unsigned char*)&x);
    load_reg_to(vm, vB, (unsigned char*)&y);
    if ( x >= y ) {
        if ( is_verbose() ) printf("if-ge v%d>=v%d (%d>=%d): branch %+d\n", vA, vB, x, y, (int)off);
        *pc = *pc + off * 2;
    } else {
        if ( is_verbose() ) printf("if-ge v%d>=v%d (%d>=%d): fall through\n", vA, vB, x, y);
        *pc = *pc + 4;
    }
    return 0;
}
//==================================================================
// 一元 / 取模 / 比較分支 opcode (OpsDvm 範例所需)
//==================================================================
// 0x7b neg-int vA, vB : vA = -vB  (12x)
int op_neg_int( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    int v = 0;
    load_reg_to(vm, vB, (unsigned char*)&v);
    v = -v;
    store_to_reg(vm, vA, (unsigned char*)&v);
    if ( is_verbose() ) printf("neg-int v%d, v%d = %d\n", vA, vB, v);
    *pc = *pc + 2;
    return 0;
}
// 0xb4 rem-int/2addr vA, vB : vA = vA %% vB  (12x)
int op_rem_int_2addr( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    int x = 0, y = 0;
    load_reg_to(vm, vA, (unsigned char*)&x);
    load_reg_to(vm, vB, (unsigned char*)&y);
    if ( y != 0 ) x = x % y;
    store_to_reg(vm, vA, (unsigned char*)&x);
    if ( is_verbose() ) printf("rem-int/2addr v%d, v%d = %d\n", vA, vB, x);
    *pc = *pc + 2;
    return 0;
}
// 0x38 if-eqz vAA, +CCCC : vAA==0 則跳 (16-bit signed, code unit)  (21t)
int op_if_eqz( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vAA = ptr[*pc+1];
    short off = (short)((ptr[*pc+3] << 8) | ptr[*pc+2]);
    int x = 0;
    load_reg_to(vm, vAA, (unsigned char*)&x);
    if ( x == 0 ) {
        if ( is_verbose() ) printf("if-eqz v%d==0: branch %+d\n", vAA, (int)off);
        *pc = *pc + off * 2;
    } else {
        if ( is_verbose() ) printf("if-eqz v%d==0: fall through\n", vAA);
        *pc = *pc + 4;
    }
    return 0;
}
// 0x36 if-gt vA, vB, +CCCC : vA>vB 則跳 (16-bit signed, code unit)  (22t)
int op_if_gt( DexFileFormat *dex, simple_dalvik_vm *vm, u1 *ptr, int *pc )
{
    int vA = ptr[*pc+1] & 0x0F;
    int vB = (ptr[*pc+1] >> 4) & 0x0F;
    short off = (short)((ptr[*pc+3] << 8) | ptr[*pc+2]);
    int x = 0, y = 0;
    load_reg_to(vm, vA, (unsigned char*)&x);
    load_reg_to(vm, vB, (unsigned char*)&y);
    if ( x > y ) {
        if ( is_verbose() ) printf("if-gt v%d>v%d (%d>%d): branch %+d\n", vA, vB, x, y, (int)off);
        *pc = *pc + off * 2;
    } else {
        if ( is_verbose() ) printf("if-gt v%d>v%d (%d>%d): fall through\n", vA, vB, x, y);
        *pc = *pc + 4;
    }
    return 0;
}
byteCode byteCodes[] = {
    { "move-result-wide"  , 0x0B, 2,  op_move_result_wide },
    { "move-result-object", 0x0C, 2,  op_move_result_object },
    { "return-void"       , 0x0e, 2,  op_return_void },
    { "const/4"           , 0x12, 2,  op_const_4 },
    { "const/16"          , 0x13, 4,  op_const_16 },
    { "const-wide/high16" , 0x19, 4,  op_const_wide_high16 },
    { "const-string"      , 0x1a, 4,  op_const_string },
    { "new-instance"      , 0x22, 4,  op_new_instance },
    { "sget-object"       , 0x62, 4,  op_sget_object },
    { "invoke-virtual"    , 0x6e, 6,  op_invoke_virtual },
    { "invoke-direct"     , 0x70, 6,  op_invoke_direct },
    { "invoke-static"     , 0x71, 6,  op_invoke_static },
    { "double-to-int"     , 0x8a, 2,  op_double_to_int},
    { "add-int"           , 0x90, 4,  op_add_int },
    { "sub-int"           , 0x91, 4,  op_sub_int },
    { "mul-int"           , 0x92, 4,  op_mul_int },
    { "div-int"           , 0x93, 4,  op_div_int },
    { "add-int/2addr"     , 0xb0, 2,  op_add_int_2addr},
    { "add-double/2addr"  , 0xcb, 2,  op_add_double_2addr},
    { "mul-double/2addr"  , 0xcd, 2,  op_mul_double_2addr},
    { "div-int/lit8"      , 0xdb, 4,  op_div_int_lit8 },
    { "new-array"         , 0x23, 4,  op_new_array },
    { "aget"              , 0x44, 4,  op_aget },
    { "aput"              , 0x4b, 4,  op_aput },
    { "move"              , 0x01, 2,  op_move },
    { "mul-int/2addr"     , 0xb2, 2,  op_mul_int_2addr },
    { "add-int/lit8"      , 0xd8, 4,  op_add_int_lit8 },
    { "goto"              , 0x28, 2,  op_goto },
    { "if-ge"             , 0x35, 4,  op_if_ge },
    { "neg-int"           , 0x7b, 2,  op_neg_int },
    { "rem-int/2addr"     , 0xb4, 2,  op_rem_int_2addr },
    { "if-eqz"            , 0x38, 4,  op_if_eqz },
    { "if-gt"             , 0x36, 4,  op_if_gt },
    { "const-wide/16"     , 0x16, 4,  op_const_wide_16 },
    { "const-wide"        , 0x18, 10, op_const_wide },
    { "aget-wide"         , 0x45, 4,  op_aget_wide },
    { "aput-wide"         , 0x4c, 4,  op_aput_wide }
};
static int byteCode_size = sizeof(byteCodes)/ sizeof(byteCode);


opCodeFunc findOpCodeFunc( unsigned char op ) {
    int i = 0;
    for ( i = 0 ; i < byteCode_size ; i++ ) {
        if ( op == byteCodes[i].opCode )
            return byteCodes[i].func;
    } 
    return 0;
}

void runMethod (DexFileFormat *dex, simple_dalvik_vm *vm, encoded_method *m )
{
    u1 *ptr =(u1*) m->code_item.insns;
    unsigned char opCode = 0;
    opCodeFunc func = 0; 

    vm->pc = 0;
    //for ( i = 0 ; i < m->code_item.insns_size * sizeof(ushort) ; ) {
    while (1) {
        //printf("opCode = %02x \n", ptr[vm->pc] );
        //printf("vm->pc = %d\n", vm->pc);
        if ( vm->pc >= m->code_item.insns_size * sizeof(ushort) )
            break;
        opCode = ptr[vm->pc];
        func= findOpCodeFunc( opCode );
        if ( func != 0 ) {
            //printRegs(vm);
            func( dex, vm, ptr, (int *)&vm->pc );
            //printRegs(vm);
        } else {
            printRegs(vm);
            printf("Unknow OpCode =%02x \n", opCode);
            break;
        }
    }
}

void simple_dvm_startup(DexFileFormat *dex, simple_dalvik_vm *vm, char *entry) 
{
    int i = 0;
    int method_name_idx = -1;
    int method_idx = -1;
    int class_idx = -1;

    method_name_idx = find_const_string(dex,entry);

    if ( method_name_idx < 0 ) {
        printf("no method %s in dex\n", entry);
        return ;
    }
    for ( i = 0 ; i < (int)dex->header.methodIdsSize; i++ ) {
        if ( (int)dex->method_id_item[i].name_idx == method_name_idx )
        {
            if ( is_verbose() > 2 ) {
                printf("find %s in class_idx[%d], method_id = %d\n", 
                        entry, i-1, i );
            }
            class_idx = i-1;
            method_idx = i;
            break;
        }
    }
    if ( class_idx < 0 || method_idx < 0 ) {
        printf("no method %s in dex\n", entry);
        return ;
    }

    encoded_method *m = 
        &dex->class_data_item[ class_idx ].direct_methods[ method_idx ];

    if ( is_verbose() > 2 ) {
        printf("encoded_method method_id = %d, insns_size = %d\n",
                m->method_idx_diff, m->code_item.insns_size );
    }

    memset(vm , 0, sizeof(simple_dalvik_vm));
    runMethod ( dex, vm, m );
    
}
