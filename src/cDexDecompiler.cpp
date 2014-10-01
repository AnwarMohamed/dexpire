#include "cDexDecompiler.h"

#define MAX_STRING_BUFFER_SIZE 200
#define MAX_DECOMPILE_BUFFER_SIZE 10000

#define OP_TYPE_V   "void"
#define OP_TYPE_Z   "boolean"
#define OP_TYPE_B   "byte"
#define OP_TYPE_S   "short"
#define OP_TYPE_C   "char"
#define OP_TYPE_I   "int"
#define OP_TYPE_J   "long"
#define OP_TYPE_F   "float"
#define OP_TYPE_D   "double"

#define Zero(b, s) memset(b, 0, s)

cDexDecompiler::cDexDecompiler(cDexFile* DexFile)
{
    Classes = 0;
    if (!DexFile->isReady) return;

    this->DexFile = DexFile;
    Classes = new DEX_DECOMPILED_CLASS[DexFile->nClasses];
    Zero(Classes, DexFile->nClasses* sizeof(DEX_DECOMPILED_CLASS));

    for (UINT i=0; i<DexFile->nClasses; i++)
    {
        Classes[i].Imports = (CHAR**)malloc(0);
        Classes[i].Extends = (CHAR**)malloc(0);
        Classes[i].Methods = (DEX_DECOMPILED_CLASS_METHOD**)malloc(0);
        DecompileClass(&Classes[i], &DexFile->DexClasses[i]);    
    }
}

void cDexDecompiler::DecompileClass(
    DEX_DECOMPILED_CLASS*Decompiled, 
    DEX_CLASS_STRUCTURE* DexClass
    )
{
    GetClassDefinition(Decompiled, DexClass);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->InstanceFieldsSize:0); i++)
        GetClassField(Decompiled,  &DexClass->ClassData->InstanceFields[i]);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->StaticFieldsSize:0); i++)
        GetClassField(Decompiled,  &DexClass->ClassData->StaticFields[i], TRUE);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->DirectMethodsSize:0); i++)
        GetClassMethod(Decompiled, &DexClass->ClassData->DirectMethods[i]);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->VirtualMethodsSize:0); i++)
        GetClassMethod(Decompiled, &DexClass->ClassData->VirtualMethods[i], TRUE);
}

void cDexDecompiler::GetClassField(
    DEX_DECOMPILED_CLASS* Decompiled, 
    CLASS_FIELD* Field,
    BOOL Static
    )
{
    DEX_DECOMPILED_CLASS_FIELD* dField = new DEX_DECOMPILED_CLASS_FIELD;
    Zero(dField, sizeof(DEX_DECOMPILED_CLASS_FIELD));

    Decompiled->Fields = (DEX_DECOMPILED_CLASS_FIELD**)realloc
        (Decompiled->Fields,++Decompiled->FieldsSize* sizeof(DEX_DECOMPILED_CLASS_FIELD*));
    Decompiled->Fields[Decompiled->FieldsSize-1] = dField;
    
    dField->Name = (CHAR*)Field->Name;
    dField->AccessFlags = ExtractAccessFlags(2, Field->AccessFlags);
    dField->Static = Static;
    dField->ReturnType = GetTypeDescription((CHAR*)Field->Type);
    dField->Value = (CHAR*)Field->Value;
}

void cDexDecompiler::AddInstructionToLine(
    DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    CLASS_CODE_INSTRUCTION* Instruction
    )
{
    Line->Instructions = (CLASS_CODE_INSTRUCTION**)realloc(Line->Instructions, ++Line->InstructionsSize*sizeof(CLASS_CODE_INSTRUCTION));
    Line->Instructions[Line->InstructionsSize-1] = Instruction;
}

void cDexDecompiler::GetClassMethod(
    DEX_DECOMPILED_CLASS* Decompiled, 
    CLASS_METHOD* Method,
    BOOL Virtual
    )
{
    DEX_DECOMPILED_CLASS_METHOD* dMethod = new DEX_DECOMPILED_CLASS_METHOD;
    Zero(dMethod, sizeof(DEX_DECOMPILED_CLASS_METHOD));

    Decompiled->Methods = (DEX_DECOMPILED_CLASS_METHOD**)realloc
        (Decompiled->Methods,++Decompiled->MethodsSize* sizeof(DEX_DECOMPILED_CLASS_METHOD*));
    Decompiled->Methods[Decompiled->MethodsSize-1] = dMethod;

    dMethod->Arguments = (DEX_DECOMPILED_CLASS_METHOD_ARGUMENT**)malloc(0);
    
    dMethod->Name = (CHAR*)Method->Name;
    //dMethod->Positions = Method->CodeArea->DebugInfo.Positions;

    if (Method->CodeArea)
        dMethod->LinesSize =  Method->CodeArea->DebugInfo.PositionsSize;

    if (dMethod->LinesSize)
        dMethod->Lines = new DEX_DECOMPILED_CLASS_METHOD_LINE*[dMethod->LinesSize];

    UINT InsIndex = 0, Size;
    for (UINT i=0; i<dMethod->LinesSize; i++)
    {
        dMethod->Lines[i] = new DEX_DECOMPILED_CLASS_METHOD_LINE;
        Zero(dMethod->Lines[i], sizeof(DEX_DECOMPILED_CLASS_METHOD_LINE));
        
        dMethod->Lines[i]->Instructions = (CLASS_CODE_INSTRUCTION**)malloc(0);

        if (i+1 != dMethod->LinesSize)
        {
            Size = Method->CodeArea->DebugInfo.Positions[i+1]->Offset - Method->CodeArea->DebugInfo.Positions[i]->Offset;
            while(InsIndex != Method->CodeArea->InstructionsSize && 
                Size >= (Method->CodeArea->Instructions[InsIndex]->BytesSize/2) && Size >0)
            {
                Size -= Method->CodeArea->Instructions[InsIndex]->BytesSize/2;
                AddInstructionToLine(dMethod->Lines[i], Method->CodeArea->Instructions[InsIndex++]);
            }
        }
        else
            while(InsIndex != Method->CodeArea->InstructionsSize)
                AddInstructionToLine(dMethod->Lines[i], Method->CodeArea->Instructions[InsIndex++]);
    }

    dMethod->ReturnType = GetTypeDescription((CHAR*)Method->ProtoType);

    AddToImports(Decompiled, dMethod->ReturnType);

    dMethod->AccessFlags = ExtractAccessFlags(1, Method->AccessFlags);
    dMethod->Virtual = Virtual;

    /* Method Arguments */
    GetClassMethodArgs(Decompiled, dMethod, Method);

    /* Method Codes */
    //GetClassMethodCodes(Decompiled, dMethod, Method);
}

void cDexDecompiler::GetClassMethodCodesLine(
    DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Registers
    )
{
    BOOL Done = FALSE;
    CHAR* Temp = new CHAR[MAX_STRING_BUFFER_SIZE];
    Zero(Temp, MAX_STRING_BUFFER_SIZE);

    for (UINT i=0; i<Line->InstructionsSize; i++)
    {
        switch(Line->Instructions[i]->Bytes[0])
        {
        case OP_NOP:

        case OP_MOVE:
        case OP_MOVE_FROM16:
        case OP_MOVE_16:
        case OP_MOVE_WIDE:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_WIDE_16:
        case OP_MOVE_OBJECT:
        case OP_MOVE_OBJECT_FROM16:
        case OP_MOVE_OBJECT_16:

        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
        case OP_MOVE_RESULT_OBJECT:
        case OP_MOVE_EXCEPTION:
            break;

        case OP_RETURN_VOID:
            sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "return");
            Done = TRUE;
            break;

        case OP_RETURN:
        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:

        case OP_CONST_4:
        case OP_CONST_16:
        case OP_CONST:
        case OP_CONST_HIGH16:
        case OP_CONST_WIDE_16:
        case OP_CONST_WIDE_32:
        case OP_CONST_WIDE:
        case OP_CONST_WIDE_HIGH16:
        case OP_CONST_STRING:
        case OP_CONST_STRING_JUMBO:
        case OP_CONST_CLASS:

        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:

        case OP_CHECK_CAST:
        case OP_INSTANCE_OF:

        case OP_ARRAY_LENGTH:

        case OP_NEW_INSTANCE:
        case OP_NEW_ARRAY:
  
        case OP_FILLED_NEW_ARRAY:
        case OP_FILLED_NEW_ARRAY_RANGE:
        case OP_FILL_ARRAY_DATA:

        case OP_THROW:
        case OP_GOTO:
        case OP_GOTO_16:
        case OP_GOTO_32:
        case OP_PACKED_SWITCH:
        case OP_SPARSE_SWITCH:
  
        case OP_CMPL_FLOAT:
        case OP_CMPG_FLOAT:
        case OP_CMPL_DOUBLE:
        case OP_CMPG_DOUBLE:
        case OP_CMP_LONG:

        case OP_IF_EQ:
        case OP_IF_NE:
        case OP_IF_LT:
        case OP_IF_GE:
        case OP_IF_GT:
        case OP_IF_LE:
        case OP_IF_EQZ:
        case OP_IF_NEZ:
        case OP_IF_LTZ:
        case OP_IF_GEZ:
        case OP_IF_GTZ:
        case OP_IF_LEZ:

        case OP_AGET:
        case OP_AGET_WIDE:
        case OP_AGET_OBJECT:
        case OP_AGET_BOOLEAN:
        case OP_AGET_BYTE:
        case OP_AGET_CHAR:
        case OP_AGET_SHORT:
        case OP_APUT:
        case OP_APUT_WIDE:
        case OP_APUT_OBJECT:
        case OP_APUT_BOOLEAN:
        case OP_APUT_BYTE:
        case OP_APUT_CHAR:
        case OP_APUT_SHORT:

        case OP_IGET:
        case OP_IGET_WIDE:
        case OP_IGET_OBJECT:
        case OP_IGET_BOOLEAN:
        case OP_IGET_BYTE:
        case OP_IGET_CHAR:
        case OP_IGET_SHORT:
        case OP_IPUT:
        case OP_IPUT_WIDE:
        case OP_IPUT_OBJECT:
        case OP_IPUT_BOOLEAN:
        case OP_IPUT_BYTE:
        case OP_IPUT_CHAR:
        case OP_IPUT_SHORT:

        case OP_SGET:
        case OP_SGET_WIDE:
        case OP_SGET_OBJECT:
        case OP_SGET_BOOLEAN:
        case OP_SGET_BYTE:
        case OP_SGET_CHAR:
        case OP_SGET_SHORT:
        case OP_SPUT:
        case OP_SPUT_WIDE:
        case OP_SPUT_OBJECT:
        case OP_SPUT_BOOLEAN:
        case OP_SPUT_BYTE:
        case OP_SPUT_CHAR:
        case OP_SPUT_SHORT:

        case OP_INVOKE_VIRTUAL:
            break;

        case OP_INVOKE_SUPER:
            Done = Line->InstructionsSize == 1;
            sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s%s(",
                Done? "super.": ".",
                DexFile->StringItems[DexFile->DexMethodIds[Line->Instructions[i]->vB].StringIndex].Data);

            sprintf_s(Temp + strlen(Temp), MAX_STRING_BUFFER_SIZE, ")");
            break;

        case OP_INVOKE_DIRECT:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_INTERFACE:

        case OP_INVOKE_VIRTUAL_RANGE:
        case OP_INVOKE_SUPER_RANGE:
        case OP_INVOKE_DIRECT_RANGE:
        case OP_INVOKE_STATIC_RANGE:
        case OP_INVOKE_INTERFACE_RANGE:

        case OP_NEG_INT:
        case OP_NOT_INT:
        case OP_NEG_LONG:
        case OP_NOT_LONG:
        case OP_NEG_FLOAT:
        case OP_NEG_DOUBLE:
        case OP_INT_TO_LONG:
        case OP_INT_TO_FLOAT:
        case OP_INT_TO_DOUBLE:
        case OP_LONG_TO_INT:
        case OP_LONG_TO_FLOAT:
        case OP_LONG_TO_DOUBLE:
        case OP_FLOAT_TO_INT:
        case OP_FLOAT_TO_LONG:
        case OP_FLOAT_TO_DOUBLE:
        case OP_DOUBLE_TO_INT:
        case OP_DOUBLE_TO_LONG:
        case OP_DOUBLE_TO_FLOAT:
        case OP_INT_TO_BYTE:
        case OP_INT_TO_CHAR:
        case OP_INT_TO_SHORT:

        case OP_ADD_INT:
        case OP_SUB_INT:
        case OP_MUL_INT:
        case OP_DIV_INT:
        case OP_REM_INT:
        case OP_AND_INT:
        case OP_OR_INT:
        case OP_XOR_INT:
        case OP_SHL_INT:
        case OP_SHR_INT:
        case OP_USHR_INT:

        case OP_ADD_LONG:
        case OP_SUB_LONG:
        case OP_MUL_LONG:
        case OP_DIV_LONG:
        case OP_REM_LONG:
        case OP_AND_LONG:
        case OP_OR_LONG:
        case OP_XOR_LONG:
        case OP_SHL_LONG:
        case OP_SHR_LONG:
        case OP_USHR_LONG:

        case OP_ADD_FLOAT:
        case OP_SUB_FLOAT:
        case OP_MUL_FLOAT:
        case OP_DIV_FLOAT:
        case OP_REM_FLOAT:
        case OP_ADD_DOUBLE:
        case OP_SUB_DOUBLE:
        case OP_MUL_DOUBLE:
        case OP_DIV_DOUBLE:
        case OP_REM_DOUBLE:

        case OP_ADD_INT_2ADDR:
        case OP_SUB_INT_2ADDR:
        case OP_MUL_INT_2ADDR:
        case OP_DIV_INT_2ADDR:
        case OP_REM_INT_2ADDR:
        case OP_AND_INT_2ADDR:
        case OP_OR_INT_2ADDR:
        case OP_XOR_INT_2ADDR:
        case OP_SHL_INT_2ADDR:
        case OP_SHR_INT_2ADDR:
        case OP_USHR_INT_2ADDR:

        case OP_ADD_LONG_2ADDR:
        case OP_SUB_LONG_2ADDR:
        case OP_MUL_LONG_2ADDR:
        case OP_DIV_LONG_2ADDR:
        case OP_REM_LONG_2ADDR:
        case OP_AND_LONG_2ADDR:
        case OP_OR_LONG_2ADDR:
        case OP_XOR_LONG_2ADDR:
        case OP_SHL_LONG_2ADDR:
        case OP_SHR_LONG_2ADDR:
        case OP_USHR_LONG_2ADDR:

        case OP_ADD_FLOAT_2ADDR:
        case OP_SUB_FLOAT_2ADDR:
        case OP_MUL_FLOAT_2ADDR:
        case OP_DIV_FLOAT_2ADDR:
        case OP_REM_FLOAT_2ADDR:
        case OP_ADD_DOUBLE_2ADDR:
        case OP_SUB_DOUBLE_2ADDR:
        case OP_MUL_DOUBLE_2ADDR:
        case OP_DIV_DOUBLE_2ADDR:
        case OP_REM_DOUBLE_2ADDR:

        case OP_ADD_INT_LIT16:
        case OP_RSUB_INT:
        case OP_MUL_INT_LIT16:
        case OP_DIV_INT_LIT16:
        case OP_REM_INT_LIT16:
        case OP_AND_INT_LIT16:
        case OP_OR_INT_LIT16:
        case OP_XOR_INT_LIT16:

        case OP_ADD_INT_LIT8:
        case OP_RSUB_INT_LIT8:
        case OP_MUL_INT_LIT8:
        case OP_DIV_INT_LIT8:
        case OP_REM_INT_LIT8:
        case OP_AND_INT_LIT8:
        case OP_OR_INT_LIT8:
        case OP_XOR_INT_LIT8:
        case OP_SHL_INT_LIT8:
        case OP_SHR_INT_LIT8:
        case OP_USHR_INT_LIT8:

        case OP_THROW_VERIFICATION_ERROR:
        case OP_EXECUTE_INLINE:
        case OP_UNUSED_EF:

        case OP_INVOKE_DIRECT_EMPTY:
        case OP_UNUSED_F1:
        case OP_IGET_QUICK:
        case OP_IGET_WIDE_QUICK:
        case OP_IGET_OBJECT_QUICK:
        case OP_IPUT_QUICK:
        case OP_IPUT_WIDE_QUICK:
        case OP_IPUT_OBJECT_QUICK:

        case OP_INVOKE_VIRTUAL_QUICK:
        case OP_INVOKE_VIRTUAL_QUICK_RANGE:
        case OP_INVOKE_SUPER_QUICK:
        case OP_INVOKE_SUPER_QUICK_RANGE:
            break;
        }

        if (Done)
        {
            Line->Decompiled = new CHAR[strlen(Temp)+1];
            memcpy(Line->Decompiled, Temp, strlen(Temp)+1);
            break;
        }
    }
    
    //delete Temp;
}

void cDexDecompiler::GetClassMethodCodes(
    DEX_DECOMPILED_CLASS* Decompiled, 
    DEX_DECOMPILED_CLASS_METHOD* dMethod,
    CLASS_METHOD* Method
    )
{
    for (UINT i=0; i<dMethod->LinesSize; i++)
        GetClassMethodCodesLine(dMethod->Lines[i], NULL);
}

UINT cDexDecompiler::GetClassMethodArgs(
    DEX_DECOMPILED_CLASS* Decompiled,
    DEX_DECOMPILED_CLASS_METHOD* dMethod, 
    CLASS_METHOD* Method    
    )
{
    UCHAR argc = 0;
    if (Method->Type &&  strlen((CHAR*)Method->Type) >0)
    {
        UINT ptr= 0;
        while(Method->Type[ptr])
        {
            switch(Method->Type[ptr])
            {
            case 'V':
            case 'Z':
            case 'B':
            case 'S':
            case 'C':
            case 'I':
            case 'J':
            case 'F':
            case 'D':
                dMethod->Arguments = (DEX_DECOMPILED_CLASS_METHOD_ARGUMENT**)
                    realloc(dMethod->Arguments, ++dMethod->ArgumentsSize*sizeof(DEX_DECOMPILED_CLASS_METHOD_ARGUMENT*));
                dMethod->Arguments[dMethod->ArgumentsSize-1] = new DEX_DECOMPILED_CLASS_METHOD_ARGUMENT;
                dMethod->Arguments[dMethod->ArgumentsSize-1]->Type = GetTypeDescription((CHAR*)&Method->Type[ptr]);

                if (Method->CodeArea && Method->CodeArea->DebugInfo.ParametersSize >argc)
                    dMethod->Arguments[dMethod->ArgumentsSize-1]->Name = (CHAR*)Method->CodeArea->DebugInfo.ParametersNames[argc];
                else
                    dMethod->Arguments[dMethod->ArgumentsSize-1]->Name = NULL;

                ptr++;  break;
            case 'L':
            case '[':
                dMethod->Arguments = (DEX_DECOMPILED_CLASS_METHOD_ARGUMENT**)
                    realloc(dMethod->Arguments, ++dMethod->ArgumentsSize*sizeof(DEX_DECOMPILED_CLASS_METHOD_ARGUMENT*));
                dMethod->Arguments[dMethod->ArgumentsSize-1] = new DEX_DECOMPILED_CLASS_METHOD_ARGUMENT;
                dMethod->Arguments[dMethod->ArgumentsSize-1]->Type = GetTypeDescription((CHAR*)&Method->Type[ptr]);

                if (Method->CodeArea && Method->CodeArea->DebugInfo.ParametersSize >argc)
                    dMethod->Arguments[dMethod->ArgumentsSize-1]->Name = (CHAR*)Method->CodeArea->DebugInfo.ParametersNames[argc];
                else
                    dMethod->Arguments[dMethod->ArgumentsSize-1]->Name = NULL;

                AddToImports(Decompiled, dMethod->Arguments[dMethod->ArgumentsSize-1]->Type); 
                
                if (Method->Type[ptr] == 'L')
                    ptr+= strlen(dMethod->Arguments[dMethod->ArgumentsSize-1]->Type) + 2;   
                else
                    ptr+= GetArrayTypeSize((CHAR*)Method->Type +ptr);

                break;
            }
        }

        return argc;
    }
    return 0;
}

UINT cDexDecompiler::GetArrayTypeSize(
    CHAR* Type
    )
{
    UINT size = 0;
    while(TRUE)
    {
        switch(*Type++)
        {
        case 'V':
        case 'Z':
        case 'B':
        case 'S':
        case 'C':
        case 'I':
        case 'J':
        case 'F':
        case 'D':
            return ++size;
        case 'L':
            while(*Type++ != ';') size++;
            return size+2;
        case '[':
            size++;
            break;
        }
    }
}

CHAR* cDexDecompiler::ExtractShortLType(CHAR* Type)
{
    if (!Type) return NULL;
    CHAR* result = strrchr(Type, '.');
    if (result)
        return result+1;
    else
        return Type;
}

CHAR* cDexDecompiler::GetShortType(
    CHAR* Type
    )
{
    return strrchr(Type, '.')?strrchr(Type, '.')+1: Type;
}

CHAR* cDexDecompiler::GetTypeDescription(
    CHAR* Type,
    UINT ArraySize
    )
{
    switch(Type[0])
    {
    case 'V':
        return ArraySize? ExtractArrayType(OP_TYPE_V, ArraySize): OP_TYPE_V;
    case 'Z':
        return ArraySize? ExtractArrayType(OP_TYPE_Z, ArraySize): OP_TYPE_Z;
    case 'B':
        return ArraySize? ExtractArrayType(OP_TYPE_B, ArraySize): OP_TYPE_B;
    case 'S':
        return ArraySize? ExtractArrayType(OP_TYPE_S, ArraySize): OP_TYPE_S;
    case 'C':
        return ArraySize? ExtractArrayType(OP_TYPE_C, ArraySize): OP_TYPE_C;
    case 'I':
        return ArraySize? ExtractArrayType(OP_TYPE_I, ArraySize): OP_TYPE_I;
    case 'J':
        return ArraySize? ExtractArrayType(OP_TYPE_J, ArraySize): OP_TYPE_J;
    case 'F':
        return ArraySize? ExtractArrayType(OP_TYPE_F, ArraySize): OP_TYPE_F;
    case 'D':
        return ArraySize? ExtractArrayType(OP_TYPE_D, ArraySize): OP_TYPE_D;
    case 'L':
        return ArraySize? ExtractArrayType(ExtractLType(Type), ArraySize): ExtractLType(Type);
    case '[':
        return GetTypeDescription(Type+1, ArraySize+1);
    default:
        return NULL;
    }
}

CHAR* cDexDecompiler::ExtractArrayType(
    CHAR* Type,
    UINT ArraySize
    )
{
    UINT resultLen = strlen(Type) + 2*ArraySize + 1;
    CHAR* result = new CHAR[resultLen];
    result[resultLen-1] = NULL;
    sprintf_s(result, resultLen, "%s", Type);
    for (UINT i=0; i<ArraySize; i++)
        sprintf_s(result + strlen(result), 3, "[]");
    return result;
}

CHAR* cDexDecompiler::ExtractLType(
    CHAR* Type
    )
{
    UINT descStrlen;
    for(descStrlen=0; Type[descStrlen]!=';'; descStrlen++);
    descStrlen++;

    CHAR* result = new CHAR[descStrlen - 1];
    result[descStrlen - 2] = NULL;
    memcpy(result, Type+1, descStrlen - 2);
    
    for (UINT i=0; i<strlen(result); i++)
        if (result[i] == '/')
            result[i] = '.';
    return result;
}

CHAR* cDexDecompiler::ExtractAccessFlags(
    CHAR Type,
    UINT AccessFlags
    )
{
    CHAR* result;
    CHAR* accessFlags = cDexFile::GetAccessMask(Type, AccessFlags);
    result = new CHAR[strlen(accessFlags) + 1];
    result[strlen(accessFlags)] = NULL;
    memcpy(result, accessFlags, strlen(accessFlags));

    for(UINT i=0; result[i] != NULL; i++)
        result[i] = tolower(result[i]);
    return result;
}

void cDexDecompiler::AddToImports(
    DEX_DECOMPILED_CLASS* Decompiled, 
    CHAR* Import
    )
{
    if (!strrchr(Import, '.')) return;
    for (UINT i=0; i<Decompiled->ImportsSize; i++)
        if (strcmp(Decompiled->Imports[i], Import) == 0)
            return;

    Decompiled->Imports = (CHAR**)realloc(Decompiled->Imports, ++Decompiled->ImportsSize* sizeof(CHAR*));
    Decompiled->Imports[Decompiled->ImportsSize-1] = Import;
}

void cDexDecompiler::AddToExtends(
    DEX_DECOMPILED_CLASS* Decompiled,
    CHAR* Superclass
    )
{
    CHAR* Super = GetTypeDescription(Superclass);
    if (*Superclass == 'L')
        AddToImports(Decompiled, Super);

    Decompiled->Extends = (CHAR**)realloc(Decompiled->Extends, ++Decompiled->ExtendsSize* sizeof(CHAR*));
    Decompiled->Extends[Decompiled->ExtendsSize-1] = Super;
}

void cDexDecompiler::GetClassDefinition(
    DEX_DECOMPILED_CLASS* Decompiled, 
    DEX_CLASS_STRUCTURE* DexClass
    )
{
    /* Class Package */
    Decompiled->Package = GetTypeDescription((CHAR*)DexClass->Descriptor);

    /* Class Name */
    Decompiled->Name = strrchr(Decompiled->Package, '.');
    *Decompiled->Name++ = NULL;

    /* Class Source FileName */
    Decompiled->SourceFile = (CHAR*)DexClass->SourceFile;

    /* Class Access Flags */
    Decompiled->AccessFlags = ExtractAccessFlags(0, DexClass->AccessFlags);

    /* Class Supoerclass */
    if (DexClass->SuperClass)
        AddToExtends(Decompiled, (CHAR*)DexClass->SuperClass);
}

cDexDecompiler::~cDexDecompiler()
{
}
