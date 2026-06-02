#include "simple_jvm.h"
#include "simple_jvm_java_lib.h"


static int run = 1;
int get_integer_parameter( StackFrame *stack, SimpleConstantPool *p)
{
    int value = 0;
    if ( is_ref_entry(stack) ) {
        int index = popInt(stack);
        value = get_integer_from_constant_pool(p, index);
    } else {
        value = popInt(stack);
    }
    return value;
}
long long get_long_parameter( StackFrame *stack, SimpleConstantPool *p)
{
    long long value = 0;
    if ( is_ref_entry(stack) ) {
        int index = popInt(stack);
        value = get_long_from_constant_pool(p, index);
    } else {
        value = popLong(stack);
    }
    return value;
}
float get_float_parameter( StackFrame *stack, SimpleConstantPool *p)
{
    float value = 0;
    if ( is_ref_entry(stack) ) {
        int index = popInt(stack);
        value = get_float_from_constant_pool(p, index);
    } else {
        value = popFloat(stack);
    }
    return value;
}
double get_double_parameter( StackFrame *stack, SimpleConstantPool *p ) 
{
    double value = 0.0f;
    if ( is_ref_entry(stack) ) {
        int index = popInt(stack);
        value = get_double_from_constant_pool(p, index);
        JVM_LOG("index %d\n", index);
        JVM_LOG("get value from constant pool = %f\n", value);
    } else {
        value = popDouble(stack);
        JVM_LOG("get value from stack = %f\n", value);
    }
    return value;
}
//--------------------------------------------------------------------------------
// aload_0
int op_aload_0( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 0);
    JVM_LOG("push 0 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
//bipush
int op_bipush( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = opCode[0][1];
    pushInt(stack, value);
    JVM_LOG("push a byte %d onto the stack \n", value);
    *opCode = *opCode + 2;
    return 0;
}
//dup
int op_dup( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    StackEntry *entry = popEntry(stack);
    int value = 0;
    value = EntryToInt(entry);
    if ( entry->type == STACK_ENTRY_INT ) {

        pushInt(stack, value);
        pushInt(stack, value);
    } else {
        pushRef(stack, value);
        pushRef(stack, value);
    }
    JVM_LOG("dup\n");
    *opCode = *opCode + 1;
    return 0;
}
//getstatic
int op_getstatic( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    u2 field_index ;
    unsigned char tmp[2];
    tmp[0] = opCode[0][1];
    tmp[1] = opCode[0][2];
    field_index = tmp[0] << 8 | tmp[1];
    JVM_LOG("getstatic %d\n", field_index );
    pushRef(stack, field_index);
    *opCode = *opCode + 3;
    return 0;
}
// iadd
int op_iadd( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value1 = popInt(stack);
    int value2 = popInt(stack);
    int result = 0;
    result = value1 + value2;
    JVM_LOG("iadd: %d + %d = %d\n",value1, value2, result);
    pushInt(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}
// iconst_0
int op_iconst_0( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 0);
    JVM_LOG("iconst_0: push 0 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
// iconst_1
int op_iconst_1( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 1);
    JVM_LOG("iconst_1: push 1 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
// iconst_2
int op_iconst_2( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 2);
    JVM_LOG("iconst_2: push 1 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
// iconst_3
int op_iconst_3( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 3);
    JVM_LOG("iconst_3: push 1 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
// iconst_4
int op_iconst_4( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 4);
    JVM_LOG("iconst_4: push 1 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
// iconst_5
int op_iconst_5( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushInt(stack, 5);
    JVM_LOG("iconst_5: push 5 into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
//0x0F dconst_1
int op_dconst_1( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    pushDouble(stack, 1.0f);
    JVM_LOG("iconst_5: push 1.0f into stack\n");
    *opCode = *opCode + 1;
    return 0;
}
// idiv
int op_idiv( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value2 = popInt(stack);
    int value1 = popInt(stack);
    int result = 0;
    result = value1 / value2;
    JVM_LOG("idiv: %d / %d = %d\n", value1, value2, result);
    pushInt(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}

//iload
int op_iload( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int index = opCode[0][1];
    int value = ctx->locals.integer[index];
    JVM_LOG("iload: load value from local variable %d(%d)\n", index, ctx->locals.integer[index]);
    pushInt(stack, value);
    *opCode = *opCode + 2;
    return 0;
}

//iload_1
int op_iload_1( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = ctx->locals.integer[1];
    JVM_LOG("iload_1: load value from local variable 1(%d)\n", ctx->locals.integer[1]);
    pushInt(stack, value);
    *opCode = *opCode + 1;
    return 0;
}

//iload_2
int op_iload_2( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = ctx->locals.integer[2];
    JVM_LOG("iload_2: load value from local variable 2(%d)\n", ctx->locals.integer[2]);
    pushInt(stack, value);
    *opCode = *opCode + 1;
    return 0;
}
//iload_3
int op_iload_3( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = ctx->locals.integer[3];
    JVM_LOG("iload_3: load value from local variable 3(%d)\n",ctx->locals.integer[3]);
    pushInt(stack, value);
    *opCode = *opCode + 1;
    return 0;
}
// imul
int op_imul( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value1 = popInt(stack);
    int value2 = popInt(stack);
    int result = 0;
    result = value1 * value2;
    JVM_LOG("imul: %d * %d = %d\n", value1, value2, result);
    pushInt(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}
//0x63 dadd
int op_dadd( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    double value1 = get_double_parameter(stack,p);
    double value2 = get_double_parameter(stack,p);
    double result = 0;
    result = value1 + value2;
    JVM_LOG("dadd: %f + %f = %f\n", value1, value2, result);
    pushDouble(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}

//0x6B dmul
int op_dmul( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    double value1 = get_double_parameter(stack,p);
    double value2 = get_double_parameter(stack,p);
    double result = 0;
    result = value1 * value2;
    JVM_LOG("dmul: %f * %f = %f\n", value1, value2, result);
    pushDouble(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}
//0x8e d2i
int op_d2i( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    double value1 = popDouble(stack);
    int result = 0;
    result = (int)value1;
    JVM_LOG("d2i: %d <-- %f\n", result, value1);
    pushInt(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}
//irem
int op_irem( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value1 = popInt(stack);
    int value2 = popInt(stack);
    int result = 0;
    result = value2 % value1;
    JVM_LOG("irem: %d %% %d = %d\n", value2, value1, result);
    pushInt(stack, result);
    *opCode = *opCode + 1;
    return 0;
}
//istore
int op_istore( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = popInt(stack);
    int index = opCode[0][1];
    JVM_LOG("istore: store value into local variable %d(%d)\n", index, value);
    ctx->locals.integer[index] = value;
    *opCode = *opCode + 2;
    return 0;
}
//istore_1
int op_istore_1( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = popInt(stack);
    JVM_LOG("istore_1: store value into local variable 1(%d)\n", value);
    ctx->locals.integer[1] = value;
    *opCode = *opCode + 1;
    return 0;
}
//istore_2
int op_istore_2( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = popInt(stack);
    JVM_LOG("istore_2: store value into local variable 2(%d)\n", value);
    ctx->locals.integer[2] = value;
    *opCode = *opCode + 1;
    return 0;
}
//istore_3
int op_istore_3( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = popInt(stack);
    JVM_LOG("istore_3: store value into local variable 3(%d)\n", value);
    ctx->locals.integer[3] = value;
    *opCode = *opCode + 1;
    return 0;
}
// isub
int op_isub( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value2 = popInt(stack);
    int value1 = popInt(stack);
    int result = 0;
    result = value1 - value2;
    JVM_LOG("isub : %d - %d = %d\n", value1, value2, result);
    pushInt(stack, result); 
    *opCode = *opCode + 1;
    return 0;
}
//invokespecial
int op_invokespecial( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    u2 method_index ;
    unsigned char tmp[2];
    tmp[0] = opCode[0][1];
    tmp[1] = opCode[0][2];
    method_index = tmp[0] << 8 | tmp[1];
    JVM_LOG("call method_index %d\n", method_index);
    *opCode = *opCode + 3;
    if ( method_index < ctx->method_pool.method_used ) {
        MethodInfo *method = &ctx->method_pool.method[method_index];
        executeMethod( ctx, method );
    }
    return 0;
}
static char *clzNamePrint="java/io/PrintStream";
static char *clzNameStrBuilder="java/lang/StringBuilder";
static char stringBuilderBuffer[1024];
static int stringBuilderUsed = 0;
//0xb8 invokestatic
int op_invokestatic( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    u2 method_index ;
    unsigned char tmp[2];
    char method_name[255];
    char clsName[255];
    char method_type[255];
    tmp[0] = opCode[0][1];
    tmp[1] = opCode[0][2];
    method_index = tmp[0] << 8 | tmp[1];
    JVM_LOG("invokestatic method_index %d\n", method_index);
    JVM_LOG("simpleMethodPool.method_used = %d\n",ctx->method_pool.method_used);
    *opCode = *opCode + 3;
    if ( method_index < ctx->method_pool.method_used ) {
        MethodInfo *method = &ctx->method_pool.method[method_index];
        memset(method_name,0,255);
        getUTF8String(p, method->name_index, 255, method_name);
        JVM_LOG(" method name = %s\n", method_name );
    } else {
        ConstantMethodRef *mRef = findMethodRef(p, method_index );
        if ( mRef != 0 ) {
            ConstantClassRef *clasz = findClassRef(p, mRef->classIndex );
            ConstantNameAndType *nat = findNameAndType(p, mRef->nameAndTypeIndex );
            if ( clasz == 0 || nat == 0 ) return -1;
            getUTF8String(p, clasz->stringIndex, 255, clsName);
            getUTF8String(p, nat->nameIndex, 255, method_name);
            getUTF8String(p, nat->typeIndex, 255, method_type);

            JVM_LOG("call class %s\n", clsName);
            JVM_LOG("call method %s\n", method_name);
            JVM_LOG("call method type %s\n", method_type);
            int ret = invoke_java_lang_library(stack,p,
                    clsName,method_name,method_type);
            (void)ret;
#if SIMPLE_JVM_DEBUG
            if ( ret ) {
                printf("invoke java lang library successful\n");
            }
#endif
        }
    }


    return 0;
}

//invokevirtual
int op_invokevirtual( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    u2 object_ref;
    unsigned char tmp[2];
    char clsName[255];
    char utf8[255];
    int len = 0;
    tmp[0] = opCode[0][1];
    tmp[1] = opCode[0][2];
    object_ref= tmp[0] << 8 | tmp[1];
    JVM_LOG("call object_ref %d\n", object_ref );
    *opCode = *opCode + 3;
    ConstantMethodRef *mRef = findMethodRef(p, object_ref );
    if ( mRef != 0 ) {
        ConstantClassRef *clasz = findClassRef(p, mRef->classIndex );
        ConstantNameAndType *nat = findNameAndType(p, mRef->nameAndTypeIndex );
        if ( clasz == 0 || nat == 0 ) return -1;
        getUTF8String(p, clasz->stringIndex, 255, clsName);
        JVM_LOG("call object ref class %s\n", clsName);
        if ( strcmp(clzNamePrint, clsName) == 0 ) {
            StackEntry *entry = popEntry(stack);
            int index = EntryToInt(entry);
            JVM_LOG("call Println with index = %d\n", index);
            if ( entry->type == STACK_ENTRY_REF ) {
                ConstantStringRef *strRef = findStringRef(p, index);
                if ( strRef != 0 ) {
                    getUTF8String(p, strRef->stringIndex, 255, utf8);
                    len = strlen(utf8);
                    memcpy( stringBuilderBuffer + stringBuilderUsed, utf8, len);
                    stringBuilderUsed += len;
                    stringBuilderBuffer[stringBuilderUsed] = 0;
                }
            } else if ( entry->type == STACK_ENTRY_INT ) {
                sprintf(utf8,"%d",index);
                len = strlen(utf8);
                memcpy( stringBuilderBuffer + stringBuilderUsed, utf8, len);
                stringBuilderUsed += len;
                stringBuilderBuffer[stringBuilderUsed] = 0;
            }
            // printf out the result
            printf("%s\n", stringBuilderBuffer); 
            memset(stringBuilderBuffer, 0, 1024);
            stringBuilderUsed = 0;
        } else if ( strcmp(clzNameStrBuilder, clsName) == 0 ){
            StackEntry *entry = popEntry(stack);
            int index = EntryToInt(entry);
            JVM_LOG("call StringBuilder with index = %d\n", index);
            if ( entry->type == STACK_ENTRY_REF ) {
                ConstantStringRef *strRef = findStringRef(p, index);
                if ( strRef != 0 ) {
                    getUTF8String(p, strRef->stringIndex, 255, utf8);
                    len = strlen(utf8);
                    memcpy( stringBuilderBuffer + stringBuilderUsed, utf8, len);
                    stringBuilderUsed += len;
                }
            } else if ( entry->type == STACK_ENTRY_INT ) {
                sprintf(utf8,"%d",index);
                len = strlen(utf8);
                memcpy( stringBuilderBuffer + stringBuilderUsed, utf8, len);
                stringBuilderUsed += len;
                JVM_LOG("%s\n", stringBuilderBuffer); 
            } 

        }
    }
    return 0;
}
//ldc
int op_ldc( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    int value = opCode[0][1];
    pushRef(stack, value);
    JVM_LOG("ldc: push a constant index %d onto the stack \n", value);
    *opCode = *opCode + 2;
    return 0;
}
//0x14 ldc2_w
int op_ldc2_w( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    unsigned char index1 = opCode[0][1];
    unsigned char index2 = opCode[0][2];
    int index = ( index1 << 8 ) | index2;
    pushRef(stack, index);
    JVM_LOG("ldc2_w: push a constant index %d onto the stack \n", index);
    *opCode = *opCode + 3;
    return 0;
}
//0x11 op_sipush
int op_sipush( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    short value;
    unsigned char tmp[2];
    tmp[0] = opCode[0][1];
    tmp[1] = opCode[0][2];
    value = tmp[0] << 8 | tmp[1];
    JVM_LOG("sipush value %d\n", value);
    pushInt(stack,value);
    *opCode = *opCode + 3;
    return 0;
}
//op_new
int op_new( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    u2 object_ref;
    unsigned char tmp[2];
    tmp[0] = opCode[0][1];
    tmp[1] = opCode[0][2];
    object_ref= tmp[0] << 8 | tmp[1];
    JVM_LOG("new: new object_ref %d\n", object_ref );
    (void)object_ref;
    *opCode = *opCode + 3;
    return 0;
}

// return
int op_return( unsigned char **opCode, StackFrame *stack, SimpleConstantPool *p, JvmContext *ctx ) {
    JVM_LOG("return: \n");
    *opCode = *opCode + 1;
    return -1;
}

byteCode byteCodes[] = {
    { "aload_0"         , 0x2A, 1,  op_aload_0          },
    { "bipush"          , 0x10, 2,  op_bipush           },
    { "dup"             , 0x59, 1,  op_dup              },
    { "getstatic"       , 0xB2, 3,  op_getstatic        },
    { "iadd"            , 0x60, 1,  op_iadd             },
    { "iconst_0"        , 0x03, 1,  op_iconst_0         },
    { "iconst_1"        , 0x04, 1,  op_iconst_1         },
    { "iconst_2"        , 0x05, 1,  op_iconst_2         },
    { "iconst_3"        , 0x06, 1,  op_iconst_3         },
    { "iconst_4"        , 0x07, 1,  op_iconst_4         },
    { "iconst_5"        , 0x08, 1,  op_iconst_5         },
    { "dconst_1"        , 0x0F, 1,  op_dconst_1         },
    { "idiv"            , 0x6C, 1,  op_idiv             },
    { "imul"            , 0x68, 1,  op_imul             },
    { "dadd"            , 0x63, 1,  op_dadd             },
    { "dmul"            , 0x6B, 1,  op_dmul             },
    { "d2i"             , 0x8e, 1,  op_d2i              },
    { "invokespecial"   , 0xB7, 3,  op_invokespecial    },
    { "invokevirtual"   , 0xB6, 3,  op_invokevirtual    },
    { "invokestatic"    , 0xB8, 3,  op_invokestatic     },
    { "iload"           , 0x15, 2,  op_iload            },
    { "iload_1"         , 0x1B, 1,  op_iload_1          },
    { "iload_2"         , 0x1C, 1,  op_iload_2          },
    { "iload_3"         , 0x1D, 1,  op_iload_3          },
    { "istore"          , 0x36, 2,  op_istore           },
    { "istore_1"        , 0x3C, 1,  op_istore_1         },
    { "istore_2"        , 0x3D, 1,  op_istore_2         },
    { "istore_3"        , 0x3E, 1,  op_istore_3         },
    { "isub"            , 0x64, 1,  op_isub             },
    { "ldc"             , 0x12, 2,  op_ldc              },
    { "ldc2_w"          , 0x14, 3,  op_ldc2_w           },
    { "new"             , 0xBB, 3,  op_new              },
    { "irem"            , 0x70, 1,  op_irem             },
    { "sipush"          , 0x11, 3,  op_sipush           },
    { "return"          , 0xB1, 1,  op_return           }
};
static int byteCode_size = sizeof(byteCodes)/ sizeof(byteCode);

char *findOpCode( unsigned char op ) {
    int i = 0;
    for ( i = 0 ; i < byteCode_size ; i++ ) {
        if ( op == byteCodes[i].opCode )
            return byteCodes[i].name;
    } 
    return 0;
}
opCodeFunc findOpCodeFunc( unsigned char op ) {
    int i = 0;
    for ( i = 0 ; i < byteCode_size ; i++ ) {
        if ( op == byteCodes[i].opCode )
            return byteCodes[i].func;
    } 
    return 0;
}
int findOpCodeOffset( unsigned char op ) {
    int i = 0;
    for ( i = 0 ; i < byteCode_size ; i++ ) {
        if ( op == byteCodes[i].opCode )
            return byteCodes[i].offset;
    } 
    return 0;
}
int convertToCodeAttribute( CodeAttribute *ca, AttributeInfo *attr) {
    int info_p = 0;
    unsigned char tmp[4];
    ca->attribute_name_index = attr->attribute_name_index;
    ca->attribute_length = attr->attribute_length;
    tmp[0] = attr->info[info_p++];
    tmp[1] = attr->info[info_p++];
    ca->max_stack = tmp[0] << 8 | tmp[1]; 
    tmp[0] = attr->info[info_p++];
    tmp[1] = attr->info[info_p++];
    ca->max_locals = tmp[0] << 8 | tmp[1]; 
    tmp[0] = attr->info[info_p++];
    tmp[1] = attr->info[info_p++];
    tmp[2] = attr->info[info_p++];
    tmp[3] = attr->info[info_p++];
    ca->code_length = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
    ca->code = (unsigned char*)malloc(sizeof(unsigned char) * ca->code_length);
    memcpy( ca->code, attr->info + info_p, ca->code_length ); 
    return 0;
}

int executeMethod( JvmContext *ctx, MethodInfo *startup ) {
    int i = 0;
    int j = 0;
    char name[255];
    CodeAttribute ca;
    memset( &ca, 0 , sizeof(CodeAttribute));
    for ( j = 0 ; j < startup->attributes_count ; j++ ) {
        convertToCodeAttribute(&ca, &startup->attributes[j]);
        getUTF8String(&ctx->constant_pool,ca.attribute_name_index,255,name);
        if ( memcmp( name, "Code", 4 ) != 0 ) continue;
#if SIMPLE_JVM_DEBUG
        printf("----------------------------------------\n");
        printf("code dump\n");
        printCodeAttribute(&ca, &ctx->constant_pool);
        printf("----------------------------------------\n");
#endif
        unsigned char *pc = ca.code;
        //printf(" run = %d\n", run );
        if ( run == 0 ) {
            exit(1);
        }
        do {
#if 1
            opCodeFunc func= findOpCodeFunc(pc[0]);
            if ( func != 0 ) {
                //printf(" pc = %d\n", pc);
                i = func( &pc , &ctx->stack, &ctx->constant_pool, ctx);
                //printf(" pc = %d\n", pc);
            }
            if ( i < 0 ) break;
#endif
        } while (1) ;
    }
    return 0;
}

void printCodeAttribute( CodeAttribute *ca, SimpleConstantPool *p ) {
    int i = 0;
    int tmp = 0;
    char name[255];
    getUTF8String(p,ca->attribute_name_index,255,name);
    printf("attribute name : %s\n", name );
    printf("attribute length: %d\n", ca->attribute_length );

    printf("max_stack: %d\n", ca->max_stack );
    printf("max_locals: %d\n", ca->max_locals );
    printf("code_length: %d\n", ca->code_length );
    unsigned char *pc = ca->code;
    i = 0;
    do {
        char *opName = findOpCode( pc[0] );
        if ( opName == 0 ) {
            printf("Unknow OpCode %02X\n", pc[0] );
            exit(1);
        }
        printf("%s \n", opName);
#if 0
        opCodeFunc func= findOpCodeFunc(pc);
        if ( func != 0 ) 
            func(opCode, &stackFrame );
#endif
        tmp = findOpCodeOffset(pc[0]);
        pc += tmp;
        i += tmp;
    } while ( i < ca->code_length ) ;
}
