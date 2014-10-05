#pragma once
#include "cDexDecompiler.h"
#include "cDexFile.h"

class DLLEXPORT cDexCodeGen
{

public:
    cDexCodeGen(cDexFile* DexFile, STRUCT DEX_DECOMPILED_CLASS* dClass, STRUCT DEX_DECOMPILED_CLASS_METHOD* dMethod, STRUCT CLASS_METHOD* Method);
    ~cDexCodeGen();

    void GenerateSourceCode();
    void DumpLineSingleInstruction(STRUCT DEX_DECOMPILED_CLASS_METHOD* dMethod, STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE* Line);

private:
    DEX_DECOMPILED_CLASS* dClass;
    DEX_DECOMPILED_CLASS_METHOD* dMethod;
    CLASS_METHOD* Method;
    cDexFile* DexFile;
};