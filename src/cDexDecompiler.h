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


STRUCT DEX_DECOMPILED_CLASS_METHOD_LINE
{
    vector<CLASS_CODE_INSTRUCTION*> Instructions;
    string Decompiled;
};

STRUCT DEX_DECOMPILED_CLASS_FIELD
{
    string  AccessFlags;
    string  Name;
    string  ReturnType;
    string  ShortReturnType;
    BOOL    Static;
    string  Value;

    CLASS_FIELD* Ref;
    STRUCT DEX_DECOMPILED_CLASS* Parent;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD_ARGUMENT
{
    string Name;
    string Type;
    string ShortType;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD_REGISTER
{
    string  Name;
    string  Value;
    string  Type;
    USHORT  StartAddress;
    USHORT  EndAddress;
    string  Signature;
    BOOL    Local;
    BOOL    Initialized;
};

STRUCT DEX_DECOMPILED_CLASS_METHOD
{
    string  AccessFlags;
    string  Name;
    string  ReturnType;
    string  ShortReturnType;
    BOOL    Virtual;

    vector<DEX_DECOMPILED_CLASS_METHOD_ARGUMENT*> Arguments;

    vector<DEX_DECOMPILED_CLASS_METHOD_LINE*> Lines;
    vector<string> Decompiled;

    map<UINT, vector<DEX_DECOMPILED_CLASS_METHOD_REGISTER*>> Registers;

    CLASS_METHOD* Ref;
    DEX_DECOMPILED_CLASS* Parent;
};

STRUCT DEX_DECOMPILED_CLASS
{
    string  Package;
    string  AccessFlags;
    string  Name;
    string  SourceFile;
    
    vector<string>  Imports;
    vector<string> Interfaces;
    vector<string> Extends;

    vector<DEX_DECOMPILED_CLASS_METHOD*> Methods;
    vector<DEX_DECOMPILED_CLASS_FIELD*> Fields;

    DEX_CLASS_STRUCTURE* Ref;
    DEX_DECOMPILED_CLASS* Parent;

    vector<DEX_DECOMPILED_CLASS*> SubClasses;
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

    vector<DEX_DECOMPILED_CLASS*> Classes;

    void    GetClassDefinition(DEX_DECOMPILED_CLASS* Class);
    void    GetClassMethod(DEX_DECOMPILED_CLASS* Class, CLASS_METHOD* Method, BOOL Virtual=FALSE);
    BOOL    AddToImports(DEX_DECOMPILED_CLASS* Class, string& Import);
    UINT    GetClassMethodArgs(DEX_DECOMPILED_CLASS_METHOD* Method);
    void    AssignArgumentName(DEX_DECOMPILED_CLASS_METHOD_ARGUMENT* &Name, UINT Index, DEX_DECOMPILED_CLASS_METHOD* &Method);
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

    DEX_DECOMPILED_CLASS_METHOD_REGISTER* AddToRegisters(UINT Index, map<UINT, DEX_DECOMPILED_CLASS_METHOD_REGISTER*> &Registers);
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* GetUnendedRegister(UINT Index, map<UINT, DEX_DECOMPILED_CLASS_METHOD_REGISTER*> &Registers);
    DEX_DECOMPILED_CLASS_METHOD_REGISTER* RestartRegister(UINT Index, map<UINT, DEX_DECOMPILED_CLASS_METHOD_REGISTER*> &Registers);
};
