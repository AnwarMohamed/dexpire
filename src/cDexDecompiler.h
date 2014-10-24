/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#pragma once
#include "cDexFile.h"
#include "cDexCodeGen.h"
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

STRUCT DEX_DECOMPILED_CLASS_METHOD_ARGUMENT
{
    CHAR*  Name;
    CHAR*  Type;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE
{
    CLASS_CODE_INSTRUCTION** Instructions;
    UINT    InstructionsSize;
    CHAR*   Decompiled;
};

STRUCT DEX_DECOMPILED_CLASS_FIELD
{
    CHAR*   AccessFlags;
    CHAR*   Name;
    CHAR*   ReturnType;
    BOOL    Static;
    CHAR*   Value;

    CLASS_FIELD* Ref;
    STRUCT DEX_DECOMPILED_CLASS* Parent;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD
{
    CHAR*   AccessFlags;
    CHAR*   Name;
    CHAR*   ReturnType;
    BOOL    Virtual;

    CLASS_CODE_LOCAL**  Arguments;
    UINT    ArgumentsSize;

    DEX_DECOMPILED_CLASS_METHOD_LINE** Lines;
    UINT    LinesSize;

    CLASS_METHOD* Ref;
    DEX_DECOMPILED_CLASS* Parent;
};

STRUCT DEX_DECOMPILED_CLASS
{
    CHAR*   Package;
    CHAR**  Imports;
    UINT    ImportsSize;

    CHAR*   AccessFlags;
    CHAR*   Name;
    CHAR*   SourceFile;
    
    CHAR**  Interfaces;
    UINT    InterfacesSize;

    CHAR**  Extends;
    UINT    ExtendsSize;

    DEX_DECOMPILED_CLASS_METHOD** Methods;
    UINT    MethodsSize;

    DEX_DECOMPILED_CLASS_FIELD** Fields;
    UINT    FieldsSize;

    DEX_CLASS_STRUCTURE* Ref;
    DEX_DECOMPILED_CLASS* Parent;

    DEX_DECOMPILED_CLASS** SubClasses;
    UINT SubClassesSize;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER
{
   CHAR* Name;
   CHAR* Value;
};

CONST CHAR ImportsBuiltIn[][20] =
{
    "java.lang.Object",
    "java.lang.String",
};

class DLLEXPORT cDexDecompiler
{
public:
    cDexDecompiler(cDexFile* dexFile);
    ~cDexDecompiler();

    DEX_DECOMPILED_CLASS* Classes;
    UINT    nClasses;

    void    GetClassDefinition(DEX_DECOMPILED_CLASS* Class);
    void    GetClassMethod(DEX_DECOMPILED_CLASS* Class, CLASS_METHOD* Method, BOOL Virtual=FALSE);
    void    AddToImports(DEX_DECOMPILED_CLASS* Class, CHAR* Import);
    UINT    GetClassMethodArgs(DEX_DECOMPILED_CLASS_METHOD* Method);
    //void    GetClassMethodCodes(DEX_DECOMPILED_CLASS* dClass, DEX_DECOMPILED_CLASS_METHOD* dMethod, CLASS_METHOD* Method, CHAR** Registers);
    

    void    GetClassField(DEX_DECOMPILED_CLASS* Class, CLASS_FIELD* Field, BOOL Static=FALSE);

    void    DecompileClass(DEX_DECOMPILED_CLASS* Class);
    void    AddToExtends(DEX_DECOMPILED_CLASS* Class, CHAR* Superclass);

    //void    GetClassMethodCodesLine(DEX_DECOMPILED_CLASS_METHOD_LINE * Line, CHAR** Registers);
    void    AddSubClass(DEX_DECOMPILED_CLASS* Class, DEX_DECOMPILED_CLASS* SubClass);
    void    AddToSubClasses(DEX_DECOMPILED_CLASS* Class, DEX_DECOMPILED_CLASS* SubClass);
    
private:
    void    AddInstructionToLine(DEX_DECOMPILED_CLASS_METHOD_LINE* Line, CLASS_CODE_INSTRUCTION* Instruction);
    cDexFile* DexFile;
};
