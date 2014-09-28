#include "cDexDecompiler.h"

#define MAX_STRING_BUFFER_SIZE 200
#define MAX_DECOMPILE_BUFFER_SIZE 10000

cDexDecompiler::cDexDecompiler(cDexFile* DexFile)
{
    singleClass = FALSE;
    tmpString = NULL;
    DecompiledClasses = new DEX_DECOMPILED_CLASS[DexFile->nClasses];
}

cDexDecompiler::cDexDecompiler(DEX_CLASS_STRUCTURE* DexClass)
{
    singleClass = TRUE;
    DecompiledClasses = new DEX_DECOMPILED_CLASS;

    GetClassDefinition(DecompiledClasses, DexClass);
    DecompiledClasses->Body += " {\n\n";

    for (UINT i=0; i<DexClass->ClassData->DirectMethodsSize; i++)
        if (strcmp((CHAR*)DexClass->ClassData->DirectMethods[i].Name, "<init>") != 0)
            GetClassMethod(DecompiledClasses, &DexClass->ClassData->DirectMethods[i]);

    for (UINT i=0; i<DexClass->ClassData->VirtualMethodsSize; i++)
    {
        DecompiledClasses->Body += "@Override\n";
        GetClassMethod(DecompiledClasses, &DexClass->ClassData->VirtualMethods[i]);
    }

    DecompiledClasses->Body += "}";
    
    printf("%s\n", DecompiledClasses->Package.c_str());
    printf("%s\n", DecompiledClasses->Imports.c_str());
    printf("%s\n", DecompiledClasses->Body.c_str());
}

void cDexDecompiler::AddToImports(
    DEX_DECOMPILED_CLASS* Decompiled, 
    UCHAR* Import
    )
{
    if (Import[0] != 'L') return;

    Decompiled->Imports += "import ";
    Decompiled->Imports += GetTypeDescription(Import);
    Decompiled->Imports += ";\n";

    if (tmpString)
    {
        delete tmpString;
        tmpString = NULL;
    }
}

void cDexDecompiler::GetClassMethod(
    DEX_DECOMPILED_CLASS* Decompiled, 
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method
    )
{
    tmpString = new CHAR[MAX_STRING_BUFFER_SIZE];
    strcpy_s(tmpString, MAX_STRING_BUFFER_SIZE, cDexFile::GetAccessMask(1, Method->AccessFlags));

    for(UINT i=0; tmpString[i] != NULL; i++)
        tmpString[i] = tolower(tmpString[i]);

    /* Method Access Flags */
    DecompiledClasses->Body += tmpString;
    
    /* Method Return Type */
    DecompiledClasses->Body += " ";
    DecompiledClasses->Body += GetTypeDescription(Method->ProtoType);
    
    /* Method Name */
    DecompiledClasses->Body += " ";
    DecompiledClasses->Body += (CHAR*)Method->Name;

    /* Method Arguments */
    DecompiledClasses->Body += '(';
    GetClassMethodArgs(Decompiled, Method);
    DecompiledClasses->Body += ") {\n";

    /* Method Codes */
    GetClassMethodCodes(Decompiled, Method);


    DecompiledClasses->Body += "}\n\n"; 

    if (tmpString)
    {
        delete tmpString;
        tmpString = NULL;
    }
}

void cDexDecompiler::GetClassMethodCodes(
    DEX_DECOMPILED_CLASS* Decompiled, 
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method
    )
{
}

UINT cDexDecompiler::GetClassMethodArgs(
    DEX_DECOMPILED_CLASS* Decompiled, 
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method    
    )
{
    UCHAR argc = 0;
    if (Method->Type && strlen((CHAR*)Method->Type) >0)
    {
        UCHAR ptr= 0;
        while(Method->Type[ptr] != NULL)
        {
            switch(Method->Type[ptr])
            {
            case 'V':
            case 'Z':
            case 'B':
            case 'S':
            case 'C':
            case 'I':
            case 'J':
            case 'F':
            case 'D':
                if (argc)  Decompiled->Body += ", ";
                tmpString = GetTypeDescription(&Method->Type[ptr]);
                Decompiled->Body += GetShortType(tmpString);
                Decompiled->Body += " arg";
                Decompiled->Body += ('0' + argc++);

                tmpString = NULL;
                ptr++;  break;
            case 'L':
                if (argc)  Decompiled->Body += ", ";
                tmpString = GetTypeDescription(&Method->Type[ptr]);
                Decompiled->Body += GetShortType(tmpString);
                Decompiled->Body += " arg";
                Decompiled->Body += ('0' + argc++);

                delete (tmpString-1);
                tmpString = NULL;

                AddToImports(Decompiled, &Method->Type[ptr]);
                ptr+= strlen((CHAR*)Method->Type +ptr);              
                break;
            case '[':
                break;
            }
        }

        return argc;
    }
    return 0;
}

CHAR* cDexDecompiler::GetShortType(
    CHAR* Type
    )
{
    return strrchr(Type, '.')?strrchr(Type, '.')+1: Type;
}

CHAR* cDexDecompiler::GetTypeDescription(
    UCHAR* Type
    )
{
    if (tmpString)
    {
        delete tmpString;
        tmpString = NULL;
    }

    switch(Type[0])
    {
    case 'V':
        return "void";
    case 'Z':
        return "boolean";
    case 'B':
        return "byte";
    case 'S':
        return "short";
    case 'C':
        return "char";
    case 'I':
        return "int";
    case 'J':
        return "long";
    case 'F':
        return "float";
    case 'D':
        return "double";
    case 'L':
        tmpString = new CHAR[MAX_STRING_BUFFER_SIZE];
        strcpy_s(tmpString, MAX_STRING_BUFFER_SIZE, (CHAR*)Type);
        *(strchr(tmpString, ';')) = NULL;

        for(UINT i=0; tmpString[i] != NULL; i++)
            if (tmpString[i] == '/')
                tmpString[i] = '.';
        return tmpString+1;
    case '[':
        return "";
    default:
        return "";
    }
}

void cDexDecompiler::GetClassDefinition(
    DEX_DECOMPILED_CLASS* Decompiled, 
    DEX_CLASS_STRUCTURE* DexClass
    )
{
    tmpString = new CHAR[MAX_STRING_BUFFER_SIZE];
    strcpy_s(tmpString, MAX_STRING_BUFFER_SIZE, cDexFile::GetAccessMask(0, DexClass->AccessFlags));

    for(UINT i=0; tmpString[i] != NULL; i++)
        tmpString[i] = tolower(tmpString[i]);
    
    Decompiled->Body += tmpString;
    Decompiled->Body += " class ";

    strcpy_s(tmpString, MAX_STRING_BUFFER_SIZE, (CHAR*)DexClass->Descriptor);
    tmpString[strlen(tmpString) -1] = NULL;
    Decompiled->Body += strrchr(tmpString, '/')+1;

    *(strrchr(tmpString, '/')) = NULL;
    for(UINT i=0; tmpString[i] != NULL; i++)
        if (tmpString[i] == '/')
            tmpString[i] = '.';
    Decompiled->Package = "package ";
    Decompiled->Package += tmpString+1;
    Decompiled->Package += ";\n";

    if (DexClass->SuperClass)
    {
        strcpy_s(tmpString, MAX_STRING_BUFFER_SIZE, (CHAR*)DexClass->SuperClass);
        tmpString[strlen(tmpString) -1] = 0;

        Decompiled->Body += " extends ";
        Decompiled->Body += strrchr(tmpString, '/') + 1;

        AddToImports(Decompiled, DexClass->SuperClass);
    }

    if (tmpString)
    {
        delete tmpString;
        tmpString = NULL;
    }
}

cDexDecompiler::~cDexDecompiler()
{
    if (tmpString)
    {
        delete tmpString;
        tmpString = NULL;
    }
}
