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

    string GetRegisterName(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** Register=0);
    STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER* GetRegister(
        UINT Index, 
        UINT InstructionIndex);
    string GetRegisterType(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** Register=0);
    string GetRegisterValue(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** Register=0);
    void GetRegisterInitialized(
        UINT Index, 
        UINT InstructionIndex,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** Register=0);
    void GetInvokeArguments(
        UINT Index,
        BOOL SkipFirst=FALSE);
    void SetRegisterValue(
        UINT Index,
        UINT InstructionIndex,
        string &Value,
        STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER** RegisterPtr=0);
    void GenrateRegistersMap();
private:
    STRUCT DEX_DECOMPILED_CLASS_METHOD* Method;
    cDexFile* DexFile;
};