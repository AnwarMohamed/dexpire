/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to Anwar Mohamed
 *  anwarelmakrahy[at]gmail.com
 *
 */

#include "cDexCodeGen.h"

cDexCodeGen::cDexCodeGen(
    cDexFile* DexFile,
    DEX_DECOMPILED_CLASS* dClass, 
    DEX_DECOMPILED_CLASS_METHOD* dMethod, 
    CLASS_METHOD* Method)
{
    this->dClass = dClass;
    this->dMethod = dMethod;
    this->Method = Method;
    this->DexFile = DexFile;
}

CHAR* cDexCodeGen::GetRegisterName(
    UINT Index,
    STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    STRUCT CLASS_CODE_REGISTER** Registers)
{
    if (Index < Method->CodeArea->RegistersSize && Registers)
    {
        CLASS_CODE_REGISTER* Register = Registers[Index];
        while(Register)
        {
            if (Register->Name && 
                Register->StartAddress <= (*Line->Instructions)->Offset &&
                Register->EndAddress >= (*Line->Instructions)->Offset)
                return Register->Name;
            Register = Register->Next;
        }
        return "// [BUG]";
    }
    return "// [BUG]";
}

CHAR* cDexCodeGen::GetRegisterValue(
    UINT Index, 
    STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    STRUCT CLASS_CODE_REGISTER** Registers)
{
    return "// [BUG]";
}

void cDexCodeGen::GenerateSourceCode()
{
    for (UINT i=0; i<dMethod->LinesSize; i++)
    {
        dMethod->Lines[i]->Decompiled = (CHAR*)malloc(MAX_DECOMPILED_STRING_SIZE);
        ZERO(dMethod->Lines[i]->Decompiled, MAX_DECOMPILED_STRING_SIZE);

        if (dMethod->Lines[i]->InstructionsSize == 1)
            DumpLineSingleInstruction(dMethod, dMethod->Lines[i], Method->CodeArea->Registers);
        else
            DumpLineMultiInstruction(dMethod, dMethod->Lines[i], Method->CodeArea->Registers);
        
        if (!strlen(dMethod->Lines[i]->Decompiled))
        {
            free(dMethod->Lines[i]->Decompiled);
            dMethod->Lines[i]->Decompiled = NULL;
        }
        else
            dMethod->Lines[i]->Decompiled = (CHAR*)realloc
              (dMethod->Lines[i]->Decompiled, strlen(dMethod->Lines[i]->Decompiled)+1);
    }
}

void cDexCodeGen::DumpLineSingleInstruction(
    DEX_DECOMPILED_CLASS_METHOD* dMethod,
    DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    CLASS_CODE_REGISTER** Registers
    )
{
    switch((*Line->Instructions)->OpcodeSig)
    {
    case OP_NOP:
        sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, "// [BUG]");
        break;

    case OP_MOVE:
    case OP_MOVE_FROM16:
    case OP_MOVE_16:
    case OP_MOVE_WIDE:
    case OP_MOVE_WIDE_FROM16:
    case OP_MOVE_WIDE_16:
        sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, "%s = %s",
            GetRegisterName((*Line->Instructions)->vA, Line, Registers),
            GetRegisterName((*Line->Instructions)->vB, Line, Registers));
        break;

    case OP_MOVE_OBJECT:
    case OP_MOVE_OBJECT_FROM16:
    case OP_MOVE_OBJECT_16:

    case OP_MOVE_RESULT:
    case OP_MOVE_RESULT_WIDE:
    case OP_MOVE_RESULT_OBJECT:
    case OP_MOVE_EXCEPTION:
        break;

    case OP_RETURN_VOID:
        if (strcmp("void", dMethod->ReturnType))
            sprintf_s(Line->Decompiled, MAX_STRING_BUFFER_SIZE, "return");
        break;

    case OP_RETURN:
    case OP_RETURN_WIDE:
    case OP_RETURN_OBJECT:
        break;

    case OP_CONST_4:
    case OP_CONST_16:
    case OP_CONST:
    case OP_CONST_HIGH16:
    case OP_CONST_WIDE_16:
    case OP_CONST_WIDE_32:
        if(Method->CodeArea->Locals->count((*Line->Instructions)->vA))
            sprintf_s(Line->Decompiled, MAX_STRING_BUFFER_SIZE, "%s = %d",
                (*Method->CodeArea->Locals)[(*Line->Instructions)->vA]->Name,
                (*Line->Instructions)->vB);
        break;
    case OP_CONST_WIDE:
    case OP_CONST_WIDE_HIGH16:
        break;

    case OP_CONST_STRING:
    case OP_CONST_STRING_JUMBO:
        if(Method->CodeArea->Locals->count((*Line->Instructions)->vA))
            sprintf_s(Line->Decompiled, MAX_STRING_BUFFER_SIZE, "%s = %s",
                (*Method->CodeArea->Locals)[(*Line->Instructions)->vA]->Name,
                DexFile->StringItems[(*Line->Instructions)->vB].Data);
        break;
        break;

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
        {
            sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                MAX_STRING_BUFFER_SIZE, "super.%s(",
                DexFile->StringItems[DexFile->DexMethodIds[(*Line->Instructions)->vB].StringIndex].Data);

            /*
            UINT ParamSize = strcmp("this", (*Method->CodeArea->Locals)[(*Line->Instructions)->vC]->Name)?0:1;  

            if ((*Line->Instructions)->vA > (0 + ParamSize))
                if(Method->CodeArea->Locals->count((*Line->Instructions)->vC))
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_STRING_BUFFER_SIZE, "%s",
                    ParamSize? (*Method->CodeArea->Locals)[(*Line->Instructions)->vD]->Name:
                    (*Method->CodeArea->Locals)[(*Line->Instructions)->vC]->Name);

            if ((*Line->Instructions)->vA > (1 + ParamSize))
                if(Method->CodeArea->Locals->count((*Line->Instructions)->vD))
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_STRING_BUFFER_SIZE, ", %s",
                    ParamSize? (*Method->CodeArea->Locals)[(*Line->Instructions)->vE]->Name:
                    (*Method->CodeArea->Locals)[(*Line->Instructions)->vD]->Name);

            if ((*Line->Instructions)->vA > (2 + ParamSize))
                if(Method->CodeArea->Locals->count((*Line->Instructions)->vE))
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_STRING_BUFFER_SIZE, ", %s",
                    ParamSize? (*Method->CodeArea->Locals)[(*Line->Instructions)->vF]->Name:
                    (*Method->CodeArea->Locals)[(*Line->Instructions)->vE]->Name);

            if ((*Line->Instructions)->vA > (3 + ParamSize))
                if(Method->CodeArea->Locals->count((*Line->Instructions)->vF))
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_STRING_BUFFER_SIZE, ", %s",
                    ParamSize? (*Method->CodeArea->Locals)[(*Line->Instructions)->vG]->Name:
                    (*Method->CodeArea->Locals)[(*Line->Instructions)->vF]->Name);

            if (!ParamSize && (*Line->Instructions)->vA == 5)
                if(Method->CodeArea->Locals->count((*Line->Instructions)->vG))
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_STRING_BUFFER_SIZE, ", %s",
                    (*Method->CodeArea->Locals)[(*Line->Instructions)->vG]->Name);
            */

            sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_STRING_BUFFER_SIZE, ")");
            break;
        }
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

void cDexCodeGen::DumpLineMultiInstruction(
    DEX_DECOMPILED_CLASS_METHOD* dMethod,
    DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
    CLASS_CODE_REGISTER** Registers
    )
{
    for (UINT j=0; j<Line->InstructionsSize; j++)
    {
        Line->Instructions[j]->Decompiled = (CHAR*)malloc(MAX_DECOMPILED_STRING_SIZE);
        ZERO(Line->Instructions[j]->Decompiled, MAX_DECOMPILED_STRING_SIZE);

        switch(LOW_BYTE(*Line->Instructions[j]->Buffer))
        {
        case OP_NOP:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "// UNKNOWN INSTRUCTION");
            break;

        case OP_MOVE:
        case OP_MOVE_FROM16:
        case OP_MOVE_16:
        case OP_MOVE_WIDE:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_WIDE_16:
        case OP_MOVE_OBJECT:
        case OP_MOVE_OBJECT_FROM16:
        case OP_MOVE_OBJECT_16:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = v%d",
                Line->Instructions[j]->vA, 
                Line->Instructions[j]->vB);
            break;

        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
        case OP_MOVE_RESULT_OBJECT:
        case OP_MOVE_EXCEPTION:
            break;

        case OP_RETURN_VOID:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "return");
            break;

        case OP_RETURN:
        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "return v%d",
                Line->Instructions[j]->vA);
            break;

        case OP_CONST_4:
        case OP_CONST_16:
        case OP_CONST:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = %d",
                Line->Instructions[j]->vA,
                Line->Instructions[j]->vB);
            break;

        case OP_CONST_HIGH16:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = %d",
                Line->Instructions[j]->vA,
                Line->Instructions[j]->vB << 16);
            break;

        case OP_CONST_WIDE_16:
        case OP_CONST_WIDE_32:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = %d",
                Line->Instructions[j]->vA,
                Line->Instructions[j]->vB);
            break;

        case OP_CONST_WIDE:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = %I64d",
                Line->Instructions[j]->vA,
                Line->Instructions[j]->vB_wide);
            break;

        case OP_CONST_WIDE_HIGH16:
            break;

        case OP_CONST_STRING:
        case OP_CONST_STRING_JUMBO:
            if (Line->Instructions[j]->vB < DexFile->nStringIDs)
                sprintf_s(
                    Line->Instructions[j]->Decompiled, 
                    MAX_DECOMPILED_STRING_SIZE, 
                    "v%d = \"%s\"",
                    Line->Instructions[j]->vA,
                    DexFile->StringItems[Line->Instructions[j]->vB].Data);
            break;

        case OP_CONST_CLASS:
            if (DexFile->DexTypeIds[Line->Instructions[j]->vB].StringIndex < DexFile->nStringIDs)
                sprintf_s(
                    Line->Instructions[j]->Decompiled, 
                    MAX_DECOMPILED_STRING_SIZE, 
                    "v%d = %s",
                    Line->Instructions[j]->vA,
                    DexFile->StringItems[DexFile->DexTypeIds[Line->Instructions[j]->vB].StringIndex].Data);
            break;

        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:

        case OP_CHECK_CAST:
        case OP_INSTANCE_OF:
            break;

        case OP_ARRAY_LENGTH:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = v%d",
                Line->Instructions[j]->vA, 
                Line->Instructions[j]->vB);
            break;

        case OP_NEW_INSTANCE:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = new %s()",
                Line->Instructions[j]->vA, 
                DexFile->StringItems[DexFile->DexTypeIds[Line->Instructions[j]->vB].StringIndex].Data);
            break;

        case OP_NEW_ARRAY:
                sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = new %s[%d]",
                Line->Instructions[j]->vA, 
                DexFile->StringItems[DexFile->DexTypeIds[Line->Instructions[j]->vC].StringIndex].Data,
                Line->Instructions[j]->vB);
            break;

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
            break;

        case OP_INVOKE_VIRTUAL:
        case OP_INVOKE_SUPER:
        case OP_INVOKE_DIRECT:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_INTERFACE:

            if (LOW_BYTE(*Line->Instructions[j]->Buffer) == OP_INVOKE_VIRTUAL)
                sprintf_s(
                    Line->Instructions[j]->Decompiled, 
                    MAX_DECOMPILED_STRING_SIZE, 
                    "%s(",
                    DexFile->StringItems[DexFile->DexMethodIds[Line->Instructions[j]->vB].StringIndex].Data);
            else if (LOW_BYTE(*Line->Instructions[j]->Buffer) == OP_INVOKE_SUPER)
                sprintf_s(
                    Line->Instructions[j]->Decompiled, 
                    MAX_DECOMPILED_STRING_SIZE, 
                    "super.%s(",
                    DexFile->StringItems[DexFile->DexMethodIds[Line->Instructions[j]->vB].StringIndex].Data);  



            if (Line->Instructions[j]->vA)
                sprintf_s(
                    Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                    "v%d", 
                    Line->Instructions[j]->vC);

            for (UINT v=1; v<Line->Instructions[j]->vA; v++)
                sprintf_s(
                    Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                    " ,v%d", 
                    Line->Instructions[j]->vArg[v]);

            sprintf_s(
                Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                ")");
            break;

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

        if (!strlen(Line->Instructions[j]->Decompiled))
        {
            free(Line->Instructions[j]->Decompiled);
            Line->Instructions[j]->Decompiled = NULL;
        }
        else
            Line->Instructions[j]->Decompiled = (CHAR*)realloc
            (Line->Instructions[j]->Decompiled, strlen(Line->Instructions[j]->Decompiled)+1);
    }
}

cDexCodeGen::~cDexCodeGen()
{
}