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
#include "cDexFile.h"
#include "cDexCodeGen.h"
#include <string>

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
    
    CHAR**  Extends;
    UINT    ExtendsSize;

    DEX_DECOMPILED_CLASS_METHOD** Methods;
    UINT    MethodsSize;

    DEX_DECOMPILED_CLASS_FIELD** Fields;
    UINT    FieldsSize;

    DEX_CLASS_STRUCTURE* Ref;
    STRUCT DEX_DECOMPILED_CLASS* Parent;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER
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

    void    GetClassDefinition(DEX_DECOMPILED_CLASS* Class);
    void    GetClassMethod(DEX_DECOMPILED_CLASS* Class, CLASS_METHOD* Method, BOOL Virtual=FALSE);
    void    AddToImports(DEX_DECOMPILED_CLASS* Class, CHAR* Import);
    UINT    GetClassMethodArgs(DEX_DECOMPILED_CLASS_METHOD* Method);
    //void    GetClassMethodCodes(DEX_DECOMPILED_CLASS* dClass, DEX_DECOMPILED_CLASS_METHOD* dMethod, CLASS_METHOD* Method, CHAR** Registers);
    

    void    GetClassField(DEX_DECOMPILED_CLASS* Class, CLASS_FIELD* Field, BOOL Static=FALSE);

    void    DecompileClass(DEX_DECOMPILED_CLASS* Class);
    void    AddToExtends(DEX_DECOMPILED_CLASS* Class, CHAR* Superclass);

    //void    GetClassMethodCodesLine(DEX_DECOMPILED_CLASS_METHOD_LINE * Line, CHAR** Registers);

    
private:
    
    

    UINT LineCounter;

    void    AddInstructionToLine(DEX_DECOMPILED_CLASS_METHOD_LINE* Line, CLASS_CODE_INSTRUCTION* Instruction);

    cDexFile* DexFile;
};

