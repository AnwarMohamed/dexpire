/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#pragma once
#include "cDexDecompiler.h"
#include "cDexFile.h"

class DLLEXPORT cDexCodeGen
{

public:
    cDexCodeGen(
        cDexFile* DexFile, 
        STRUCT DEX_DECOMPILED_CLASS_METHOD* Method);
    ~cDexCodeGen();

    void GenerateSourceCode();
    void DumpLineSingleInstruction(
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line, 
        STRUCT CLASS_CODE_REGISTER** Registers);
    void DumpLineMultiInstruction(
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line, 
        STRUCT CLASS_CODE_REGISTER** Registers);

    CHAR* GetRegisterName(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
        STRUCT CLASS_CODE_REGISTER** Registers);
    CHAR* GetRegisterType(
        UINT Index, 
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
        STRUCT CLASS_CODE_REGISTER** Registers);
    CHAR* GetRegisterValue(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
        STRUCT CLASS_CODE_REGISTER** Registers);
    BOOL GetRegisterInitialized(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
        STRUCT CLASS_CODE_REGISTER** Registers);
    void GetInvokeArguments(
        UINT Index,
        DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
        CLASS_CODE_REGISTER** Registers,
        BOOL SkipFirst=FALSE);
    void SetRegisterValue(
        UINT Index,
        UINT InstructionIndex,
        CHAR* Value,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line,
        STRUCT CLASS_CODE_REGISTER** Registers);

private:
    struct DEX_DECOMPILED_CLASS_METHOD* Method;
    cDexFile* DexFile;
};