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

#pragma once
#include "cDexDecompiler.h"
#include "cDexFile.h"

class DLLEXPORT cDexCodeGen
{

public:
    cDexCodeGen(
        cDexFile* DexFile, 
        STRUCT DEX_DECOMPILED_CLASS* dClass, 
        STRUCT DEX_DECOMPILED_CLASS_METHOD* dMethod, 
        STRUCT CLASS_METHOD* Method);
    ~cDexCodeGen();

    void GenerateSourceCode();
    void DumpLineSingleInstruction(
        STRUCT DEX_DECOMPILED_CLASS_METHOD* dMethod, 
        STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line, 
        STRUCT CLASS_CODE_REGISTER** Registers);
    void DumpLineMultiInstruction(
        STRUCT DEX_DECOMPILED_CLASS_METHOD* dMethod, 
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
    DEX_DECOMPILED_CLASS* dClass;
    DEX_DECOMPILED_CLASS_METHOD* dMethod;
    CLASS_METHOD* Method;
    cDexFile* DexFile;
};