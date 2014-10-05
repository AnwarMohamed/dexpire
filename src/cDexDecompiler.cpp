#include "cDexDecompiler.h"

cDexDecompiler::cDexDecompiler(cDexFile* DexFile)
{
    Classes = NULL;
    nClasses = 0;
    if (!DexFile->isReady) return;

    LineCounter = 0;

    this->DexFile = DexFile;
    Classes = new DEX_DECOMPILED_CLASS[DexFile->nClasses];
    ZERO(Classes, DexFile->nClasses* sizeof(DEX_DECOMPILED_CLASS));

    for (UINT i=0; i<DexFile->nClasses; i++)
    {
        Classes[i].Imports = (CHAR**)malloc(0);
        Classes[i].Extends = (CHAR**)malloc(0);
        Classes[i].Methods = (DEX_DECOMPILED_CLASS_METHOD**)malloc(0);
        DecompileClass(&Classes[i], &DexFile->DexClasses[i]);    
    }
}

void cDexDecompiler::DecompileClass(
    DEX_DECOMPILED_CLASS*dClass, 
    DEX_CLASS_STRUCTURE* DexClass
    )
{
    GetClassDefinition(dClass, DexClass);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->InstanceFieldsSize:0); i++)
        GetClassField(dClass,  &DexClass->ClassData->InstanceFields[i]);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->StaticFieldsSize:0); i++)
        GetClassField(dClass,  &DexClass->ClassData->StaticFields[i], TRUE);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->DirectMethodsSize:0); i++)
        GetClassMethod(dClass, &DexClass->ClassData->DirectMethods[i]);

    for (UINT i=0; i<(DexClass->ClassData?DexClass->ClassData->VirtualMethodsSize:0); i++)
        GetClassMethod(dClass, &DexClass->ClassData->VirtualMethods[i], TRUE);
}

void cDexDecompiler::GetClassField(
    DEX_DECOMPILED_CLASS* dClass, 
    CLASS_FIELD* Field,
    BOOL Static
    )
{
    DEX_DECOMPILED_CLASS_FIELD* dField = new DEX_DECOMPILED_CLASS_FIELD;
    ZERO(dField, sizeof(DEX_DECOMPILED_CLASS_FIELD));

    dClass->Fields = (DEX_DECOMPILED_CLASS_FIELD**)realloc
        (dClass->Fields,++dClass->FieldsSize* sizeof(DEX_DECOMPILED_CLASS_FIELD*));
    dClass->Fields[dClass->FieldsSize-1] = dField;
    
    dField->Name = (CHAR*)Field->Name;
    dField->AccessFlags = cDexString::ExtractAccessFlags(2, Field->AccessFlags);
    dField->Static = Static;
    dField->ReturnType = cDexString::GetTypeDescription((CHAR*)Field->Type);
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
    DEX_DECOMPILED_CLASS* dClass, 
    CLASS_METHOD* Method,
    BOOL Virtual
    )
{
    DEX_DECOMPILED_CLASS_METHOD* dMethod = new DEX_DECOMPILED_CLASS_METHOD;
    ZERO(dMethod, sizeof(DEX_DECOMPILED_CLASS_METHOD));

    dClass->Methods = (DEX_DECOMPILED_CLASS_METHOD**)realloc
        (dClass->Methods,++dClass->MethodsSize* sizeof(DEX_DECOMPILED_CLASS_METHOD*));
    dClass->Methods[dClass->MethodsSize-1] = dMethod;
    
    dMethod->Name = (CHAR*)Method->Name;

    if (Method->CodeArea)
        dMethod->LinesSize =  Method->CodeArea->DebugInfo.PositionsSize;

    LineCounter++;
    if (dMethod->LinesSize)
        dMethod->Lines = new DEX_DECOMPILED_CLASS_METHOD_LINE*[dMethod->LinesSize];

    UINT InsIndex = 0, Size;
    for (UINT i=0; i<dMethod->LinesSize; i++)
    {
        dMethod->Lines[i] = new DEX_DECOMPILED_CLASS_METHOD_LINE;
        ZERO(dMethod->Lines[i], sizeof(DEX_DECOMPILED_CLASS_METHOD_LINE));
        
        dMethod->Lines[i]->Instructions = (CLASS_CODE_INSTRUCTION**)malloc(0);

        if (i+1 != dMethod->LinesSize)
        {
            Size = Method->CodeArea->DebugInfo.Positions[i+1]->Offset - Method->CodeArea->DebugInfo.Positions[i]->Offset;
            while(InsIndex != Method->CodeArea->InstructionsSize && 
                Size >= (UINT)(Method->CodeArea->Instructions[InsIndex]->BytesSize/2) && Size >0)
            {
                Size -= Method->CodeArea->Instructions[InsIndex]->BytesSize/2;
                AddInstructionToLine(dMethod->Lines[i], Method->CodeArea->Instructions[InsIndex++]);
            }
        }
        else
            while(InsIndex != Method->CodeArea->InstructionsSize)
                AddInstructionToLine(dMethod->Lines[i], Method->CodeArea->Instructions[InsIndex++]);
    }

    dMethod->ReturnType = cDexString::GetTypeDescription((CHAR*)Method->ProtoType);

    AddToImports(dClass, dMethod->ReturnType);

    dMethod->AccessFlags = cDexString::ExtractAccessFlags(1, Method->AccessFlags);
    dMethod->Virtual = Virtual;

    /* Method Arguments */
    GetClassMethodArgs(dClass, dMethod, Method);

    /* Method Codes */
    if (Method->CodeArea)
    {
        DEX_DECOMPILED_CLASS_METHOD_REGISTER* Registers = NULL;
        cDexCodeGen* CodeGenerator = NULL;
        if (Method->CodeArea->RegistersSize)
        {
            Registers = new DEX_DECOMPILED_CLASS_METHOD_REGISTER[Method->CodeArea->RegistersSize];
            ZERO(Registers, Method->CodeArea->RegistersSize* sizeof(DEX_DECOMPILED_CLASS_METHOD_REGISTER));
        }

        CodeGenerator = new cDexCodeGen(DexFile, dClass, dMethod, Method);
        CodeGenerator->GenerateSourceCode();
        delete CodeGenerator;

        if (Registers)
            delete Registers;
    }
}

UINT cDexDecompiler::GetClassMethodArgs(
    DEX_DECOMPILED_CLASS* dClass,
    DEX_DECOMPILED_CLASS_METHOD* dMethod, 
    CLASS_METHOD* Method    
    )
{
    if (!Method->CodeArea || !Method->CodeArea->Locals || !Method->CodeArea->Locals->size() ||
        Method->CodeArea->Locals->size() > Method->CodeArea->RegistersSize)
        return 0;

    dMethod->ArgumentsSize = Method->CodeArea->Locals->size();

    map<UINT, CLASS_CODE_LOCAL*>::iterator LocalsIterator = Method->CodeArea->Locals->begin();

    if (Method->CodeArea->Locals->begin()->second->Name &&
        strcmp("this", Method->CodeArea->Locals->begin()->second->Name) == 0)
    {
        dMethod->ArgumentsSize--;
        LocalsIterator++;
    }

    if (!dMethod->ArgumentsSize) return 0;
    dMethod->Arguments = (CLASS_CODE_LOCAL**)malloc(dMethod->ArgumentsSize*sizeof(CLASS_CODE_LOCAL*));
    for (UINT LocalsForIndex=0; LocalsIterator!=Method->CodeArea->Locals->end(); ++LocalsIterator, ++LocalsForIndex)
    {
        dMethod->Arguments[LocalsForIndex] = LocalsIterator->second;
        AddToImports(dClass, LocalsIterator->second->Type); 
    }

    return dMethod->ArgumentsSize;
}

void cDexDecompiler::AddToImports(
    DEX_DECOMPILED_CLASS* dClass, 
    CHAR* Import
    )
{
    if (!strrchr(Import, '.')) return;
    for (UINT i=0; i<dClass->ImportsSize; i++)
        if (strcmp(dClass->Imports[i], Import) == 0)
            return;

    dClass->Imports = (CHAR**)realloc(dClass->Imports, ++dClass->ImportsSize* sizeof(CHAR*));
    dClass->Imports[dClass->ImportsSize-1] = Import;
}

void cDexDecompiler::AddToExtends(
    DEX_DECOMPILED_CLASS* dClass,
    CHAR* Superclass
    )
{
    CHAR* Super = cDexString::GetTypeDescription(Superclass);
    if (*Superclass == 'L')
        AddToImports(dClass, Super);

    dClass->Extends = (CHAR**)realloc(dClass->Extends, ++dClass->ExtendsSize* sizeof(CHAR*));
    dClass->Extends[dClass->ExtendsSize-1] = Super;
}

void cDexDecompiler::GetClassDefinition(
    DEX_DECOMPILED_CLASS* dClass, 
    DEX_CLASS_STRUCTURE* DexClass
    )
{
    /* Class Package */
    dClass->Package = cDexString::GetTypeDescription((CHAR*)DexClass->Descriptor);

    /* Class Name */
    dClass->Name = strrchr(dClass->Package, '.');
    *dClass->Name++ = NULL;

    /* Class Source FileName */
    dClass->SourceFile = (CHAR*)DexClass->SourceFile;

    /* Class Access Flags */
    dClass->AccessFlags = cDexString::ExtractAccessFlags(0, DexClass->AccessFlags);

    /* Class Superclass */
    if (DexClass->SuperClass)
        AddToExtends(dClass, (CHAR*)DexClass->SuperClass);
}

/*
void cDexDecompiler::GetClassMethodCodesLine(
    DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    CHAR** Registers
    )
{
    Line->dClass = (CHAR*)malloc(MAX_DECOMPILED_STRING_SIZE);
    ZERO(Line->dClass, MAX_DECOMPILED_STRING_SIZE);

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
            sprintf_s(Line->dClass, MAX_STRING_BUFFER_SIZE, "return");
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
            //Done = Line->InstructionsSize == 1;
            //sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s%s(",
            //    Done? "super.": ".",
            //    DexFile->StringItems[DexFile->DexMethodIds[Line->Instructions[i]->vB].StringIndex].Data);
            //
            //sprintf_s(Temp + strlen(Temp), MAX_STRING_BUFFER_SIZE, ")");
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
    }

    if (!strlen(Line->dClass))
    {
        free(Line->dClass);
        Line->dClass = NULL;
    }
    else
        Line->dClass = (CHAR*)realloc(Line->dClass, strlen(Line->dClass)+1);
}
*/
cDexDecompiler::~cDexDecompiler()
{
}
