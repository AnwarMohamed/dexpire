#pragma once
#include "dexpire.h"
#include <string>

#define OP_TYPE_V   "void"
#define OP_TYPE_Z   "boolean"
#define OP_TYPE_B   "byte"
#define OP_TYPE_S   "short"
#define OP_TYPE_C   "char"
#define OP_TYPE_I   "int"
#define OP_TYPE_J   "long"
#define OP_TYPE_F   "float"
#define OP_TYPE_D   "double"

static const CHAR* AccessMaskStrings[3][18] = 
{
    {   
        /* class, inner class */
        "PUBLIC",           /* 0x0001 */
        "PRIVATE",          /* 0x0002 */
        "PROTECTED",        /* 0x0004 */
        "STATIC",           /* 0x0008 */
        "FINAL",            /* 0x0010 */
        "?",                /* 0x0020 */
        "?",                /* 0x0040 */
        "?",                /* 0x0080 */
        "?",                /* 0x0100 */
        "INTERFACE",        /* 0x0200 */
        "ABSTRACT",         /* 0x0400 */
        "?",                /* 0x0800 */
        "SYNTHETIC",        /* 0x1000 */
        "ANNOTATION",       /* 0x2000 */
        "ENUM",             /* 0x4000 */
        "?",                /* 0x8000 */
        "VERIFIED",         /* 0x10000 */
        "OPTIMIZED",        /* 0x20000 */
    },
    {
        /* method */
        "PUBLIC",           /* 0x0001 */
        "PRIVATE",          /* 0x0002 */
        "PROTECTED",        /* 0x0004 */
        "STATIC",           /* 0x0008 */
        "FINAL",            /* 0x0010 */
        "SYNCHRONIZED",     /* 0x0020 */
        "BRIDGE",           /* 0x0040 */
        "VARARGS",          /* 0x0080 */
        "NATIVE",           /* 0x0100 */
        "?",                /* 0x0200 */
        "ABSTRACT",         /* 0x0400 */
        "STRICT",           /* 0x0800 */
        "SYNTHETIC",        /* 0x1000 */
        "?",                /* 0x2000 */
        "?",                /* 0x4000 */
        "MIRANDA",          /* 0x8000 */
        "CONSTRUCTOR",      /* 0x10000 */
        "DECLARED_SYNCHRONIZED", /* 0x20000 */
    },
    {
        /* field */
        "PUBLIC",           /* 0x0001 */
        "PRIVATE",          /* 0x0002 */
        "PROTECTED",        /* 0x0004 */
        "STATIC",           /* 0x0008 */
        "FINAL",            /* 0x0010 */
        "?",                /* 0x0020 */
        "VOLATILE",         /* 0x0040 */
        "TRANSIENT",        /* 0x0080 */
        "?",                /* 0x0100 */
        "?",                /* 0x0200 */
        "?",                /* 0x0400 */
        "?",                /* 0x0800 */
        "SYNTHETIC",        /* 0x1000 */
        "?",                /* 0x2000 */
        "ENUM",             /* 0x4000 */
        "?",                /* 0x8000 */
        "?",                /* 0x10000 */
        "?",                /* 0x20000 */
    },
};


class DLLEXPORT cDexString
{
public:
    cDexString();
    ~cDexString();

    static CHAR* GetTypeDescription(CHAR* Type, UINT ArraySize=0);
    static CHAR* GetAccessMask(UINT Type, UINT AccessFlag);
    static UINT  GetArrayTypeSize(CHAR* Type);
    static CHAR* GetShortType(CHAR* Type);

    static CHAR* ExtractLType(CHAR* Type);
    static CHAR* ExtractArrayType(CHAR* Type, UINT ArraySize);
    static CHAR* ExtractAccessFlags(CHAR Type, UINT AccessFlags);
    static CHAR* ExtractShortLType(CHAR* Type);
};