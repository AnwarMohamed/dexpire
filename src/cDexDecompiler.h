#pragma once
#include "cDexFile.h"
#include <string>

using namespace std;

struct DEX_DECOMPILED_CLASS
{
    CHAR*   Package;
    CHAR**  Imports;
    UINT    ImportsSize;

    CHAR*   AccessFlags;
    CHAR*   Name;
    CHAR*   SourceFile;
    
    CHAR**  Extends;
    UINT    ExtendsSize;

    struct DEX_DECOMPILED_CLASS_METHOD
    {
        CHAR*   AccessFlags;
        CHAR*   Name;
        CHAR*   ReturnType;
        BOOL    Virtual;
        struct DEX_DECOMPILED_CLASS_METHOD_ARGUMENT
        {
            CHAR*  Name;
            CHAR*  Type;
        } **  Arguments;
        UINT    ArgumentsSize;
    } ** Methods;
    UINT    MethodsSize;
};

class DLLEXPORT cDexDecompiler
{
public:
    cDexDecompiler(cDexFile* dexFile);
    ~cDexDecompiler();

    DEX_DECOMPILED_CLASS* Classes;

    void    GetClassDefinition(DEX_DECOMPILED_CLASS* Decompiled, DEX_CLASS_STRUCTURE* DexClass);
    void    GetClassMethod(DEX_DECOMPILED_CLASS* Decompiled, CLASS_METHOD* Method, BOOL Virtual=FALSE);
    void    AddToImports(DEX_DECOMPILED_CLASS* Decompiled, CHAR* Import);
    UINT    GetClassMethodArgs(DEX_DECOMPILED_CLASS* Decompiled, DEX_DECOMPILED_CLASS::DEX_DECOMPILED_CLASS_METHOD* dMethod, CLASS_METHOD* Method);
    void    GetClassMethodCodes(DEX_DECOMPILED_CLASS* Decompiled, CLASS_METHOD* Method);
    CHAR*   GetShortType(CHAR* Type);

    void    DecompileClass(DEX_DECOMPILED_CLASS* Decompiled, DEX_CLASS_STRUCTURE* DexClass);
    void    AddToExtends(DEX_DECOMPILED_CLASS* Decompiled, CHAR* Superclass);

    static CHAR* ExtractShortLType(CHAR* Type);
private:
    CHAR* GetTypeDescription(CHAR* Type);
    CHAR* ExtractLType(CHAR* Type);
    CHAR* ExtractAccessFlags(CHAR Type, UINT AccessFlags);
};

