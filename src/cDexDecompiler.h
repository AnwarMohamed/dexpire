#pragma once
#include "cDexFile.h"
#include <string>

using namespace std;

struct DEX_DECOMPILED_CLASS
{
    string Body;
    string Imports;
    string Package;
};

class DLLEXPORT cDexDecompiler
{
public:
    cDexDecompiler(cDexFile* dexFile);
    cDexDecompiler(DEX_CLASS_STRUCTURE* dexClass);
    
    ~cDexDecompiler();

    DEX_DECOMPILED_CLASS* DecompiledClasses;

    void GetClassDefinition(DEX_DECOMPILED_CLASS* Decompiled, DEX_CLASS_STRUCTURE* DexClass);
    void GetClassMethod(DEX_DECOMPILED_CLASS* Decompiled, DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method);
    void AddToImports(DEX_DECOMPILED_CLASS* Decompiled, UCHAR* Import);
    UINT GetClassMethodArgs(DEX_DECOMPILED_CLASS* Decompiled, DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method);
    void GetClassMethodCodes(DEX_DECOMPILED_CLASS* Decompiled, DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method);
    CHAR* GetShortType(CHAR* Type);
private:
    BOOL singleClass;
    CHAR* tmpString;
    INT   tmpInt;

    CHAR* GetTypeDescription(UCHAR* Type);
};

