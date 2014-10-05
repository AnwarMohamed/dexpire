#pragma once
#include "cDexFile.h"
#include "cDexCodeGen.h"
#include <string>

#define MAX_STRING_BUFFER_SIZE 200
#define MAX_DECOMPILE_BUFFER_SIZE 10000
#define MAX_DECOMPILED_STRING_SIZE  400

using namespace std;

struct DEX_DECOMPILED_CLASS_METHOD_ARGUMENT
{
    CHAR*  Name;
    CHAR*  Type;
};

struct DEX_DECOMPILED_CLASS_METHOD_LINE
{
    CLASS_CODE_INSTRUCTION** Instructions;
    UINT    InstructionsSize;
    CHAR*   Decompiled;
};

struct DEX_DECOMPILED_CLASS_FIELD
{
    CHAR*   AccessFlags;
    CHAR*   Name;
    CHAR*   ReturnType;
    BOOL    Static;
    CHAR*   Value;
};

struct DEX_DECOMPILED_CLASS_METHOD
{
    CHAR*   AccessFlags;
    CHAR*   Name;
    CHAR*   ReturnType;
    BOOL    Virtual;

    CLASS_CODE_LOCAL**  Arguments;
    UINT    ArgumentsSize;

    DEX_DECOMPILED_CLASS_METHOD_LINE** Lines;
    UINT    LinesSize;
};

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

    DEX_DECOMPILED_CLASS_METHOD** Methods;
    UINT    MethodsSize;

    DEX_DECOMPILED_CLASS_FIELD** Fields;
    UINT    FieldsSize;
};

struct DEX_DECOMPILED_CLASS_METHOD_REGISTER
{
   CHAR* Name;
   CHAR* Value;
};

class DLLEXPORT cDexDecompiler
{
public:
    cDexDecompiler(cDexFile* dexFile);
    ~cDexDecompiler();

    DEX_DECOMPILED_CLASS* Classes;
    UINT    nClasses;

    void    GetClassDefinition(DEX_DECOMPILED_CLASS* dClass, DEX_CLASS_STRUCTURE* DexClass);
    void    GetClassMethod(DEX_DECOMPILED_CLASS* dClass, CLASS_METHOD* Method, BOOL Virtual=FALSE);
    void    AddToImports(DEX_DECOMPILED_CLASS* dClass, CHAR* Import);
    UINT    GetClassMethodArgs(DEX_DECOMPILED_CLASS* dClass, DEX_DECOMPILED_CLASS_METHOD* dMethod, CLASS_METHOD* Method);
    //void    GetClassMethodCodes(DEX_DECOMPILED_CLASS* dClass, DEX_DECOMPILED_CLASS_METHOD* dMethod, CLASS_METHOD* Method, CHAR** Registers);
    

    void    GetClassField(DEX_DECOMPILED_CLASS* dClass, CLASS_FIELD* Field, BOOL Static=FALSE);

    void    DecompileClass(DEX_DECOMPILED_CLASS* dClass, DEX_CLASS_STRUCTURE* DexClass);
    void    AddToExtends(DEX_DECOMPILED_CLASS* dClass, CHAR* Superclass);

    //void    GetClassMethodCodesLine(DEX_DECOMPILED_CLASS_METHOD_LINE * Line, CHAR** Registers);

    
private:
    
    

    UINT LineCounter;

    void    AddInstructionToLine(DEX_DECOMPILED_CLASS_METHOD_LINE* Line, CLASS_CODE_INSTRUCTION* Instruction);

    cDexFile* DexFile;
};

