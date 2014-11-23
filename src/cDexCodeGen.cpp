/*
*
*  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
*  This file is subject to the terms and conditions defined in
*  file 'LICENSE.txt', which is part of this source code package.
*
*/

#include "cDexCodeGen.h"
#include <sstream>

cDexCodeGen::cDexCodeGen(
    cDexFile* DexFile,
    DEX_DECOMPILED_CLASS_METHOD* Method)
{
    this->Method = Method;
    this->DexFile = DexFile;
}

void cDexCodeGen::SetRegisterValue(
    UINT Index,
    UINT InstructionIndex,
    string &Value,
    STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** RegisterPtr)
{
    for (UINT i=0; i<Method->Registers[Index].size(); i++)
    {
        if (Method->Registers[Index][i]->StartAddress <= Method->Ref->CodeArea->Instructions[InstructionIndex]->Offset+1 &&
            Method->Registers[Index][i]->EndAddress >= Method->Ref->CodeArea->Instructions[InstructionIndex]->Offset)
        {
            if (RegisterPtr) *RegisterPtr = Method->Registers[Index][i];
            Method->Registers[Index][i]->Value = Value;
            return;
        }
    }

    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register = new DEX_DECOMPILED_CLASS_METHOD_REGISTER;
    ZERO(Register, sizeof(DEX_DECOMPILED_CLASS_METHOD_REGISTER));

    Register->StartAddress = 0;
    Register->EndAddress = Method->Ref->CodeArea->InstBufferSize;
    Register->Value = Value;
    Method->Registers[Index].push_back(Register);

    if (RegisterPtr) *RegisterPtr = Register;
}

void cDexCodeGen::GetRegisterInitialized(
    UINT Index,
    UINT InstructionIndex,
    STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** RegisterPtr)
{
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register = GetRegister(Index, InstructionIndex);
    if (Register)
    {
        if (RegisterPtr) *RegisterPtr = Register;
        Register->Initialized = TRUE;
    }
}

string cDexCodeGen::GetRegisterName(
    UINT Index,
    UINT InstructionIndex,
    STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** RegisterPtr)
{
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register = GetRegister(Index, InstructionIndex);
    if (Register)
    {
        if (RegisterPtr) *RegisterPtr = Register;
        return Register->Name;
    }
    return NULL;
}

string cDexCodeGen::GetRegisterType(
    UINT Index,
    UINT InstructionIndex,
    STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** RegisterPtr)
{
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register = GetRegister(Index, InstructionIndex);
    if (Register)
    {
        if (RegisterPtr) *RegisterPtr = Register;
        return Register->Type;
    }
    return NULL;
}

STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER* cDexCodeGen::GetRegister(
    UINT Index, 
    UINT InstructionIndex
    )
{
    if (Method->Registers[Index].size() == 1)
        return Method->Registers[Index][0];

    for (UINT i=0; i<Method->Registers[Index].size(); i++)
    {
        if (Method->Registers[Index][i]->StartAddress <= Method->Ref->CodeArea->Instructions[InstructionIndex]->Offset+1 &&
            Method->Registers[Index][i]->EndAddress >= Method->Ref->CodeArea->Instructions[InstructionIndex]->Offset)
        {
            return Method->Registers[Index][i];
        }
    }
    return NULL;
}

string cDexCodeGen::GetRegisterValue(
    UINT Index, 
    UINT InstructionIndex,
    STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** RegisterPtr)
{
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register = GetRegister(Index, InstructionIndex);
    if (Register)
    {
        if (RegisterPtr) *RegisterPtr = Register;
        return Register->Value;
    }
    return NULL;
}

void cDexCodeGen::GenrateRegistersMap()
{
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register;
    CLASS_CODE_LOCAL* Local;
    for(map<UINT, vector<CLASS_CODE_LOCAL*>>::iterator iterator = Method->Ref->CodeArea->Locals.begin(); 
        iterator != Method->Ref->CodeArea->Locals.end(); iterator++) 
    {
        for (UINT i=0; i<iterator->second.size(); i++)
        {
            Register = new DEX_DECOMPILED_CLASS_METHOD_REGISTER;
            ZERO(Register, sizeof(DEX_DECOMPILED_CLASS_METHOD_REGISTER));

            Local = iterator->second[i];

            Register->Name = Local->Name;
            Register->Type = Local->Type;
            Register->EndAddress = Local->EndAddress;
            Register->StartAddress = Local->StartAddress;
            Register->Signature = Local->Signature;
            Register->Local = TRUE;

            Method->Registers[iterator->first].push_back(Register);
        }
    }

    //if (Method->Arguments.size() == 0)
    //    return;

    USHORT RegisterIndex = Method->Ref->CodeArea->RegistersSize - Method->Ref->CodeArea->InsSize;
    
    BOOL EntryFound = FALSE;
    if (!(Method->Ref->AccessFlags & ACC_STATIC))
    {
        for (UINT i=0; i<Method->Registers[RegisterIndex].size(); i++)
        {
            if (Method->Registers[RegisterIndex][i]->Name == "this")
            {
                EntryFound = TRUE;
                break;
            }
        }

        if (!EntryFound)
        {
            Register = new DEX_DECOMPILED_CLASS_METHOD_REGISTER;
            ZERO(Register, sizeof(DEX_DECOMPILED_CLASS_METHOD_REGISTER));

            Register->Name = "this";
            Register->Type = Method->ReturnType;
            Register->EndAddress = Method->Ref->CodeArea->InstBufferSize;
            Register->StartAddress = 0;
            Register->Local = TRUE;

            Method->Registers[RegisterIndex++].push_back(Register);
        }
    }

    for (UINT i=0; i<Method->Arguments.size(); i++)
    {
        EntryFound = FALSE;
        for (UINT j=0; j<Method->Registers[RegisterIndex].size(); j++)
        {
            if (Method->Registers[RegisterIndex][j]->Name == Method->Arguments[i]->Name)
            {
                EntryFound = TRUE;
                break;
            }
        }

        if (!EntryFound)
        {
            Register = new DEX_DECOMPILED_CLASS_METHOD_REGISTER;
            ZERO(Register, sizeof(DEX_DECOMPILED_CLASS_METHOD_REGISTER));

            Register->Name = Method->Arguments[i]->Name;
            Register->Type = Method->Arguments[i]->Type;
            Register->EndAddress = Method->Ref->CodeArea->InstBufferSize;
            Register->StartAddress = 0;
            Register->Local = TRUE;

            Method->Registers[RegisterIndex++].push_back(Register);
        }
    }

    for(UINT i=0; i<Method->Ref->CodeArea->RegistersSize; i++) 
    {
        if (Method->Registers.count(i))
        {
            if (Method->Registers[i].size() == 1)
                Method->Registers[i][0]->StartAddress = 0;
        }
        else
        {
            Register = new DEX_DECOMPILED_CLASS_METHOD_REGISTER;
            ZERO(Register, sizeof(DEX_DECOMPILED_CLASS_METHOD_REGISTER));

            Register->EndAddress = Method->Ref->CodeArea->InstBufferSize;
            Register->StartAddress = 0;

            Method->Registers[i].push_back(Register);
        }
    }
}

void cDexCodeGen::GenerateSourceCode()
{
    CLASS_CODE_INSTRUCTION* Instruction;
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* TempRegister;
    for (UINT i=0; i<Method->Ref->CodeArea->Instructions.size(); i++)
    {
        Instruction = Method->Ref->CodeArea->Instructions[i];
        switch(Instruction->OpcodeSig)
        {
        case OP_NOP:
            break;

        case OP_MOVE:
        case OP_MOVE_FROM16:
        case OP_MOVE_16:
        case OP_MOVE_WIDE:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_WIDE_16:
                break;

        case OP_MOVE_OBJECT:
        case OP_MOVE_OBJECT_FROM16:
        case OP_MOVE_OBJECT_16:

        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
            break;

        case OP_MOVE_RESULT_OBJECT:
            SetRegisterValue(
                Instruction->vA, 
                i, 
                Method->Ref->CodeArea->Instructions[i-1]->Decompiled,
                &TempRegister);

            if (i+1 == Method->Ref->CodeArea->Instructions.size() ||
                Method->Ref->CodeArea->Instructions[i+1]->OpcodeSig != OP_CHECK_CAST)
            {
                if (TempRegister->Local)
                {
                    Instruction->Decompiled.clear();

                    if (!TempRegister->Initialized)
                    {
                        Instruction->Decompiled += cDexString::GetShortType((CHAR*)TempRegister->Type.c_str());
                        Instruction->Decompiled += " ";
                        TempRegister->Initialized = TRUE;
                    }

                    Instruction->Decompiled += TempRegister->Name;
                    Instruction->Decompiled += " = ";
                    Instruction->Decompiled += TempRegister->Value;

                    Method->Decompiled.push_back(Instruction->Decompiled);
                }
                else
                {
                    Instruction->Decompiled += TempRegister->Value;
                }               
            }
            break;

        case OP_MOVE_EXCEPTION:
            break;

        case OP_RETURN_VOID:
            break;

        case OP_RETURN:
            break;

        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:
            break;

        case OP_CONST_4:
        case OP_CONST_16:
        case OP_CONST:
        case OP_CONST_HIGH16:
            {
                TempRegister = GetRegister(Instruction->vA, i);
                stringstream ValueStream;

                if (Instruction->OpcodeSig == OP_CONST_HIGH16)
                    ValueStream << (Instruction->vB << 16);
                else if (Instruction->OpcodeSig == OP_CONST_WIDE)
                    ValueStream << Instruction->vB_wide;
                else
                    ValueStream << Instruction->vB;

                TempRegister->Value = ValueStream.str();

                if (TempRegister->Local)
                {
                    string Output = TempRegister->Name;
                    Output += " = ";
                    Output += TempRegister->Value;

                    Method->Decompiled.push_back(Output);
                }
                break;
            }
        case OP_CONST_WIDE_16:
        case OP_CONST_WIDE_32:          
                break;

        case OP_CONST_WIDE:
        case OP_CONST_WIDE_HIGH16:
            break;

        case OP_CONST_STRING:
        case OP_CONST_STRING_JUMBO:   
            if (Instruction->vB < DexFile->nStringIDs)
            {
                Instruction->Decompiled = "\"";
                if (DexFile->StringItems[Instruction->vB].Data)
                    Instruction->Decompiled +=  (CHAR*)DexFile->StringItems[Instruction->vB].Data;
                Instruction->Decompiled += "\"";

                SetRegisterValue(Instruction->vA, i, Instruction->Decompiled, &TempRegister);

                if (TempRegister->Local)
                {
                    string Output;

                    if (!TempRegister->Initialized)
                    {
                        Output += cDexString::GetShortType((CHAR*)TempRegister->Type.c_str());
                        Output += " ";
                        TempRegister->Initialized = TRUE;
                    }

                    Output += TempRegister->Name;
                    Output += " = ";
                    Output += Instruction->Decompiled;

                    Method->Decompiled.push_back(Output);
                }
            }
            break;

        case OP_CONST_CLASS:

        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:
            break;

        case OP_CHECK_CAST:
            TempRegister = GetRegister(Instruction->vA, i);
            Instruction->Decompiled += '(';
            Instruction->Decompiled+= cDexString::ExtractShortLType(cDexString::GetTypeDescription(
                            (CHAR*)DexFile->StringItems[DexFile->DexTypeIds[Instruction->vB].StringIndex].Data));
            Instruction->Decompiled += ')';
            Instruction->Decompiled += TempRegister->Value;
            TempRegister->Value = Instruction->Decompiled;

            if (TempRegister->Local)
            {
                Instruction->Decompiled.clear();

                if (!TempRegister->Initialized)
                {
                    Instruction->Decompiled += cDexString::GetShortType((CHAR*)TempRegister->Type.c_str());
                    Instruction->Decompiled += " ";
                    TempRegister->Initialized = TRUE;
                }

                Instruction->Decompiled += TempRegister->Name;
                Instruction->Decompiled += " = ";
                Instruction->Decompiled += TempRegister->Value;
                Method->Decompiled.push_back(Instruction->Decompiled);
            }
            break;

        case OP_INSTANCE_OF:

        case OP_ARRAY_LENGTH:
            break;

        case OP_NEW_INSTANCE:
            TempRegister = GetRegister(Instruction->vA, i);
            TempRegister->Value = "new ";
            TempRegister->Value += cDexString::ExtractShortLType(
                                        cDexString::GetTypeDescription(
                                        (CHAR*)DexFile->StringItems[DexFile->DexTypeIds[
                                            Instruction->vB
                                        ].StringIndex].Data));
            /*
            if (TempRegister->Local)
            {
                Instruction->Decompiled = TempRegister->Name;
                Instruction->Decompiled += " = ";
                Instruction->Decompiled += TempRegister->Value;
                Method->Decompiled.push_back(Instruction->Decompiled);
            }
            */

            break;

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
            break;

        case OP_INVOKE_VIRTUAL:
        case OP_INVOKE_DIRECT:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_INTERFACE:
        case OP_INVOKE_SUPER:
            if (Instruction->OpcodeSig == OP_INVOKE_SUPER)
            {
                Instruction->Decompiled += "super.";
            }
            else if (Instruction->OpcodeSig == OP_INVOKE_VIRTUAL)
            {
                TempRegister = GetRegister(Instruction->vC, i);
                if (TempRegister)
                {
                    Instruction->Decompiled += TempRegister->Local? TempRegister->Name: TempRegister->Value;
                    Instruction->Decompiled += '.';
                }
            }
            else if (Instruction->OpcodeSig == OP_INVOKE_STATIC)
            {
                Instruction->Decompiled +=
                    cDexString::ExtractShortLType(
                        cDexString::GetTypeDescription(
                            (CHAR*)DexFile->StringItems[DexFile->DexTypeIds[DexFile->DexMethodIds[
                                Instruction->vB
                            ].ClassIndex].StringIndex].Data));
                Instruction->Decompiled += '.';
            }

            if (Instruction->OpcodeSig == OP_INVOKE_DIRECT)
            {
                TempRegister = GetRegister(Instruction->vC, i);
                if (TempRegister)
                {
                    //Instruction->Decompiled += TempRegister->Local? TempRegister->Name: TempRegister->Value;
                    Instruction->Decompiled += TempRegister->Value;
                    Instruction->Decompiled += '(';             
                }
            }
            else
            {
                if (DexFile->StringItems[DexFile->DexMethodIds[Instruction->vB].StringIndex].Data)
                    Instruction->Decompiled += (CHAR*)DexFile->StringItems[DexFile->DexMethodIds[Instruction->vB].StringIndex].Data;
                Instruction->Decompiled += '(';
            }

            GetInvokeArguments(i, Instruction->OpcodeSig == OP_INVOKE_STATIC? FALSE:TRUE);  

            Instruction->Decompiled += ')';                      
            
            if (Instruction->OpcodeSig == OP_INVOKE_DIRECT)
            {
                TempRegister = GetRegister(Instruction->vC, i);
                TempRegister->Value = Instruction->Decompiled;

                if (TempRegister->Local)
                {
                    Instruction->Decompiled.clear();

                    if (!TempRegister->Initialized)
                    {
                        Instruction->Decompiled += cDexString::GetShortType((CHAR*)TempRegister->Type.c_str());
                        Instruction->Decompiled += " ";
                        TempRegister->Initialized = TRUE;
                    }

                    Instruction->Decompiled += TempRegister->Name;
                    Instruction->Decompiled += " = ";
                    Instruction->Decompiled += TempRegister->Value;

                    Method->Decompiled.push_back(Instruction->Decompiled);
                }
            }
            else if (i+1 == Method->Ref->CodeArea->Instructions.size() ||
                Method->Ref->CodeArea->Instructions[i+1]->OpcodeSig != OP_MOVE_RESULT_OBJECT)
                Method->Decompiled.push_back(Instruction->Decompiled);
            
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
    }

    /*
    for (UINT i=0; i<Method->LinesSize; i++)
    {
        Method->Lines[i]->Decompiled = (CHAR*)malloc(MAX_DECOMPILED_STRING_SIZE);
        ZERO(Method->Lines[i]->Decompiled, MAX_DECOMPILED_STRING_SIZE);

        if (Method->Lines[i]->InstructionsSize == 1)
            DumpLineSingleInstruction(Method->Lines[i], Method->Ref->CodeArea->Registers);
        else
            DumpLineMultiInstruction(Method->Lines[i], Method->Ref->CodeArea->Registers);

        if (!strlen(Method->Lines[i]->Decompiled))
        {
            free(Method->Lines[i]->Decompiled);
            Method->Lines[i]->Decompiled = NULL;
        }
        else
            Method->Lines[i]->Decompiled = (CHAR*)realloc
            (Method->Lines[i]->Decompiled, strlen(Method->Lines[i]->Decompiled)+1);
    }
    */
}

void cDexCodeGen::GetInvokeArguments(
    UINT Index,
    BOOL SkipFirst
    )
{
    if (!SkipFirst)
        Method->Ref->CodeArea->Instructions[Index]->Decompiled +=  
            GetRegisterValue(Method->Ref->CodeArea->Instructions[Index]->vC, Index);

    DEX_DECOMPILED_CLASS_METHOD_REGISTER* Register;
    for (INT i=0; i<(INT)(Method->Ref->CodeArea->Instructions[Index]->vA-1); i++)
    {
        if (i || !SkipFirst)
            Method->Ref->CodeArea->Instructions[Index]->Decompiled += ", ";

        Register = GetRegister(Method->Ref->CodeArea->Instructions[Index]->vArg[i], Index);
        if (Register)
            Method->Ref->CodeArea->Instructions[Index]->Decompiled +=  
                Register->Local? Register->Name: Register->Value;
    }
}

/*
    switch(Line->Instructions[0]->OpcodeSig)
    {
    case OP_NOP:
        Line->Decompiled = "[BUG]";
        break;

    case OP_MOVE:
    case OP_MOVE_FROM16:
    case OP_MOVE_16:
    case OP_MOVE_WIDE:
    case OP_MOVE_WIDE_FROM16:
    case OP_MOVE_WIDE_16:
        {
            BOOL RegInit = GetRegisterInitialized((*Line->Instructions)->vA, 0, Line, Registers);
            sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, "%s%s%s = %s",
                RegInit? "": GetRegisterType((*Line->Instructions)->vA, Line, Registers),
                RegInit? "": " ",
                GetRegisterName((*Line->Instructions)->vA, 0, Line, Registers),
                GetRegisterName((*Line->Instructions)->vB, 0, Line, Registers));
            break;
        }

    case OP_MOVE_OBJECT:
    case OP_MOVE_OBJECT_FROM16:
    case OP_MOVE_OBJECT_16:

    case OP_MOVE_RESULT:
    case OP_MOVE_RESULT_WIDE:
    case OP_MOVE_RESULT_OBJECT:
    case OP_MOVE_EXCEPTION:
        break;

    case OP_RETURN_VOID:
        if (strcmp("void", Method->ReturnType))
            sprintf_s(Line->Decompiled, MAX_STRING_BUFFER_SIZE, "return");
        break;

    case OP_RETURN:
        break;

    case OP_RETURN_WIDE:
    case OP_RETURN_OBJECT:
        break;

    case OP_CONST_4:
    case OP_CONST_16:
    case OP_CONST:
    case OP_CONST_HIGH16:
    case OP_CONST_WIDE_16:
    case OP_CONST_WIDE_32:
        {
            BOOL RegInit = GetRegisterInitialized((*Line->Instructions)->vA, 0, Line, Registers);
            sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, "%s%s%s = %d",
                RegInit? "": GetRegisterType((*Line->Instructions)->vA, Line, Registers),
                RegInit? "": " ",
                GetRegisterName((*Line->Instructions)->vA, 0, Line, Registers),
                (*Line->Instructions)->vB);
            break;
        }
    case OP_CONST_WIDE:
    case OP_CONST_WIDE_HIGH16:
        break;

    case OP_CONST_STRING:
    case OP_CONST_STRING_JUMBO:
        if(Method->Ref->CodeArea->Locals->count((*Line->Instructions)->vA))
            sprintf_s(Line->Decompiled, MAX_STRING_BUFFER_SIZE, "%s = %s",
            (*Method->Ref->CodeArea->Locals)[(*Line->Instructions)->vA]->Name,
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
        sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, 
            "%s.%s(",
            Registers[(*Line->Instructions)->vC]->Name,
            DexFile->StringItems[DexFile->DexMethodIds[(*Line->Instructions)->vB].StringIndex].Data);            
        GetInvokeArguments(0, Line, Registers, TRUE);            
        sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), ")");
        break;

    case OP_INVOKE_SUPER:
        sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, 
            "super.%s(",
            DexFile->StringItems[DexFile->DexMethodIds[(*Line->Instructions)->vB].StringIndex].Data);            
        GetInvokeArguments(0, Line, Registers, TRUE);            
        sprintf_s(Line->Decompiled + strlen(Line->Decompiled), MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), ")");
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
    */

    /*
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* TempRegister= NULL;
    BOOL LastInstruction;
    for (UINT j=0; j<Line->InstructionsSize; j++)
    {
        Line->Instructions[j]->Decompiled = (CHAR*)malloc(MAX_DECOMPILED_STRING_SIZE);
        ZERO(Line->Instructions[j]->Decompiled, MAX_DECOMPILED_STRING_SIZE);

        LastInstruction = (j+1== Line->InstructionsSize);

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
            break;

        case OP_MOVE_RESULT_OBJECT:
            SetRegisterValue(
                Line->Instructions[j]->vA, 
                j, 
                Line->Instructions[j-1]->Decompiled,
                Line,
                Registers,
                &TempRegister);

            if (TempRegister->Local)
                sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "(%s = %s)",
                TempRegister->Name, 
                Line->Instructions[j-1]->Decompiled);

            if (LastInstruction)
            {
                if (!TempRegister->Initialized)
                {
                    TempRegister->Initialized = TRUE;
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                        MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                        "%s ",
                        cDexString::ExtractShortLType(TempRegister->Type)); 
                }
                sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                    "%s = %s",
                    TempRegister->Name,
                    TempRegister->Value);    
            }
            break;

        case OP_MOVE_EXCEPTION:
            break;

        case OP_RETURN_VOID:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "return");
            break;

        case OP_RETURN:
            {
                sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                    "return ");

                if (!strcmp(Method->ReturnType, "boolean"))
                {
                    if (GetRegisterValue(Line->Instructions[j]->vA, j, Line, Registers))
                        sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                        MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                        "%s",
                        atol(GetRegisterValue(Line->Instructions[j]->vA, j, Line, Registers))? "true":"false");
                }
                break;
            }

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
        case OP_CONST_HIGH16:
        case OP_CONST_WIDE:
            {
                CHAR* TempString = (CHAR*)malloc(MAX_STRING_BUFFER_SIZE);
                ZERO(TempString, MAX_STRING_BUFFER_SIZE);

                if (Line->Instructions[j]->OpcodeSig == OP_CONST_HIGH16)
                    _itoa_s(Line->Instructions[j]->vB << 16, TempString, MAX_STRING_BUFFER_SIZE, 10);
                else if (Line->Instructions[j]->OpcodeSig == OP_CONST_WIDE)
                    _snprintf_s(TempString, MAX_STRING_BUFFER_SIZE, MAX_STRING_BUFFER_SIZE, "%ulld", Line->Instructions[j]->vB_wide);
                else
                    _itoa_s(Line->Instructions[j]->vB, TempString, MAX_STRING_BUFFER_SIZE, 10);

                TempString = (CHAR*)realloc(TempString, strlen(TempString) + 1);

                SetRegisterValue(Line->Instructions[j]->vA, j, TempString, Line, Registers);
                break;
            }

        case OP_CONST_WIDE_16:
        case OP_CONST_WIDE_32:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "v%d = %d",
                Line->Instructions[j]->vA,
                Line->Instructions[j]->vB);
            break;

        case OP_CONST_WIDE_HIGH16:
            break;

        case OP_CONST_STRING:
        case OP_CONST_STRING_JUMBO:       
            if (Line->Instructions[j]->vB < DexFile->nStringIDs)
            {
                CHAR* Value = new CHAR[DexFile->StringItems[Line->Instructions[j]->vB].StringSize +3];
                sprintf_s(
                    Value,
                    DexFile->StringItems[Line->Instructions[j]->vB].StringSize +3,
                    "\"%s\"",
                    DexFile->StringItems[Line->Instructions[j]->vB].Data);

                SetRegisterValue(Line->Instructions[j]->vA, j, Value, Line, Registers);
            }
            break;

        case OP_CONST_CLASS:
            if (DexFile->DexTypeIds[Line->Instructions[j]->vB].StringIndex < DexFile->nStringIDs)
                sprintf_s(Line->Instructions[j]->Decompiled, MAX_DECOMPILED_STRING_SIZE, 
                    "v%d = %s",
                    Line->Instructions[j]->vA,
                    DexFile->StringItems[DexFile->DexTypeIds[Line->Instructions[j]->vB].StringIndex].Data);
            break;

        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:
            break;

        case OP_CHECK_CAST:
            TempRegister = GetRegister(Line->Instructions[j]->vA, j, Line, Registers); 
            if (LastInstruction)
                sprintf_s(Line->Decompiled, MAX_DECOMPILED_STRING_SIZE, 
                    "%s = (%s)%s",
                    TempRegister->Name,
                    cDexString::ExtractShortLType(
                        cDexString::GetTypeDescription(
                            (CHAR*)DexFile->StringItems[DexFile->DexTypeIds[Line->Instructions[j]->vB].StringIndex].Data)),
                    TempRegister->Value);   
            else
            {
            }
            break;

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
            {
                CHAR* Type = cDexString::ExtractShortLType(
                                cDexString::GetTypeDescription(
                                    (CHAR*)DexFile->StringItems[DexFile->DexTypeIds[
                                        Line->Instructions[j]->vB
                                    ].StringIndex].Data));
                CHAR* Value = new CHAR[strlen(Type)+ 5];

                sprintf_s(Value, strlen(Type)+ 5, "new %s", Type);
                SetRegisterValue(Line->Instructions[j]->vA, j, Value, Line, Registers);
            }
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
            break;

        case OP_IGET_OBJECT:
            sprintf_s(
                Line->Instructions[j]->Decompiled, 
                MAX_DECOMPILED_STRING_SIZE, 
                "%s.%s",
                GetRegisterValue(
                    Line->Instructions[j]->vB,
                    j,
                    Line,
                    Registers),
                    DexFile->StringItems[DexFile->DexFieldIds[Line->Instructions[j]->vC].StringIndex].Data);

            SetRegisterValue(
                Line->Instructions[j]->vA, 
                j, 
                Line->Instructions[j]->Decompiled, 
                Line, 
                Registers);       
            break;

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

            if (Line->Instructions[j]->OpcodeSig == OP_INVOKE_SUPER)
                sprintf_s(Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                    "super.");

            else if (Line->Instructions[j]->OpcodeSig == OP_INVOKE_VIRTUAL)
            {
                TempRegister = GetRegister(Line->Instructions[j]->vC, j, Line, Registers);
                if (TempRegister)
                    sprintf_s(Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                        MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                        "%s.",
                        TempRegister->Local? TempRegister->Name: TempRegister->Value);
            }

            else if (Line->Instructions[j]->OpcodeSig == OP_INVOKE_STATIC)
                sprintf_s(Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                    "%s.",
                    cDexString::ExtractShortLType(
                        cDexString::GetTypeDescription(
                            (CHAR*)DexFile->StringItems[DexFile->DexTypeIds[DexFile->DexMethodIds[
                                Line->Instructions[j]->vB
                            ].ClassIndex].StringIndex].Data)));

            if (Line->Instructions[j]->OpcodeSig == OP_INVOKE_DIRECT)
            {
                TempRegister = GetRegister(Line->Instructions[j]->vC, j, Line, Registers);
                sprintf_s(Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                    "%s(",
                    TempRegister->Value); 
            }

            else
                sprintf_s(Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                    MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                    "%s(",
                    DexFile->StringItems[DexFile->DexMethodIds[Line->Instructions[j]->vB].StringIndex].Data); 

            GetInvokeArguments(j, Line, Registers, Line->Instructions[j]->OpcodeSig == OP_INVOKE_STATIC? FALSE:TRUE);  

            sprintf_s(Line->Instructions[j]->Decompiled + strlen(Line->Instructions[j]->Decompiled), 
                MAX_DECOMPILED_STRING_SIZE - strlen(Line->Instructions[j]->Decompiled), 
                ")");

            if (LastInstruction)
                if (Line->Instructions[j]->OpcodeSig == OP_INVOKE_DIRECT)
                {
                    if (TempRegister && TempRegister->Local)
                    {
                        if (!TempRegister->Initialized)
                        {
                            TempRegister->Initialized = TRUE;
                            sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                                MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                                "%s ",
                                cDexString::ExtractShortLType(TempRegister->Type)); 
                        }
                        sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                            MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                            "%s = %s",
                            TempRegister->Name,
                            Line->Instructions[j]->Decompiled); 
                    }
                    else
                        sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                            MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                            "%s",
                            Line->Instructions[j]->Decompiled); 
                }
                else 
                {
                    sprintf_s(Line->Decompiled + strlen(Line->Decompiled), 
                        MAX_DECOMPILED_STRING_SIZE - strlen(Line->Decompiled), 
                        "%s",                        
                        Line->Instructions[j]->Decompiled); 
                }
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
    */

cDexCodeGen::~cDexCodeGen()
{
}