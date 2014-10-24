/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#include "cDexString.h"

BOOL cDexString::StartsWith(CONST CHAR *pre, CONST CHAR *str)
{
    if (!pre || !str)
        return FALSE;

    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? FALSE : strncmp(pre, str, lenpre) == 0;
}

UINT cDexString::GetArrayTypeSize(
    CHAR* Type
    )
{
    UINT size = 0;
    while(TRUE)
    {
        switch(*Type++)
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
            return ++size;
        case 'L':
            while(*Type++ != ';') size++;
            return size+2;
        case '[':
            size++;
            break;
        }
    }
}

CHAR* cDexString::ExtractShortLType(CHAR* Type)
{
    if (!Type) return NULL;
    CHAR* result = strrchr(Type, '.');
    if (result)
        return result+1;
    else
        return Type;
}

CHAR* cDexString::GetShortType(
    CHAR* Type
    )
{
    return strrchr(Type, '.')?strrchr(Type, '.')+1: Type;
}

CHAR* cDexString::GetAccessMask(
    UINT Type, 
    UINT AccessFlags
    )
{
    CHAR* str,* cp;

    if (!Type && (AccessFlags & 0x0200))
        AccessFlags &= ~0x0400;

    UINT flag = AccessFlags;

    UINT flagsCount = 0;
    while (flag != 0) 
    {
        flag &= flag-1;
        flagsCount++;
    }

    cp = str = (CHAR*) malloc(flagsCount*22 +1);
    for (UINT i = 0; i < 18; i++) 
    {
        if (AccessFlags & 0x01) 
        {
            if (cp != str)
                *cp++ = ' ';
            memcpy(cp, AccessMaskStrings[Type][i], strlen(AccessMaskStrings[Type][i]));
            cp += strlen(AccessMaskStrings[Type][i]);
        }
        AccessFlags >>= 1;
    }

    *cp = '\0';
    return str;
}


CHAR* cDexString::ExtractAccessFlags(
    CHAR Type,
    UINT AccessFlags
    )
{
    CHAR* result;
    CHAR* accessFlags = GetAccessMask(Type, AccessFlags);
    result = new CHAR[strlen(accessFlags) + 1];
    result[strlen(accessFlags)] = NULL;
    memcpy(result, accessFlags, strlen(accessFlags));

    for(UINT i=0; result[i] != NULL; i++)
        result[i] = tolower(result[i]);
    return result;
}

CHAR* cDexString::GetTypeDescription(
    CHAR* Type,
    UINT ArraySize
    )
{
    switch(Type[0])
    {
    case 'V':
        return ArraySize? ExtractArrayType(OP_TYPE_V, ArraySize): OP_TYPE_V;
    case 'Z':
        return ArraySize? ExtractArrayType(OP_TYPE_Z, ArraySize): OP_TYPE_Z;
    case 'B':
        return ArraySize? ExtractArrayType(OP_TYPE_B, ArraySize): OP_TYPE_B;
    case 'S':
        return ArraySize? ExtractArrayType(OP_TYPE_S, ArraySize): OP_TYPE_S;
    case 'C':
        return ArraySize? ExtractArrayType(OP_TYPE_C, ArraySize): OP_TYPE_C;
    case 'I':
        return ArraySize? ExtractArrayType(OP_TYPE_I, ArraySize): OP_TYPE_I;
    case 'J':
        return ArraySize? ExtractArrayType(OP_TYPE_J, ArraySize): OP_TYPE_J;
    case 'F':
        return ArraySize? ExtractArrayType(OP_TYPE_F, ArraySize): OP_TYPE_F;
    case 'D':
        return ArraySize? ExtractArrayType(OP_TYPE_D, ArraySize): OP_TYPE_D;
    case 'L':
        return ArraySize? ExtractArrayType(ExtractLType(Type), ArraySize): ExtractLType(Type);
    case '[':
        return GetTypeDescription(Type+1, ArraySize+1);
    default:
        return NULL;
    }
}

CHAR* cDexString::ExtractArrayType(
    CHAR* Type,
    UINT ArraySize
    )
{
    UINT resultLen = strlen(Type) + 2*ArraySize + 1;
    CHAR* result = new CHAR[resultLen];
    result[resultLen-1] = NULL;
    sprintf_s(result, resultLen, "%s", Type);
    for (UINT i=0; i<ArraySize; i++)
        sprintf_s(result + strlen(result), 3, "[]");
    return result;
}

CHAR* cDexString::ExtractLType(
    CHAR* Type
    )
{
    UINT descStrlen;
    for(descStrlen=0; Type[descStrlen]!=';'; descStrlen++);
    descStrlen++;

    CHAR* result = new CHAR[descStrlen - 1];
    result[descStrlen - 2] = NULL;
    memcpy(result, Type+1, descStrlen - 2);
    
    for (UINT i=0; i<strlen(result); i++)
        if (result[i] == '/')
            result[i] = '.';
    return result;
}
