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
#include "dexpire.h"
#include "cFile.h"
#include "cDexString.h"
#include <map>

#define NO_INDEX 0xffffffff
#define ZERO(b, s) memset(b, 0, s)

#define MAX_STRING_BUFFER_SIZE 200
#define MAX_DECOMPILE_BUFFER_SIZE 10000
#define MAX_DECOMPILED_STRING_SIZE  4000

#define LOW_BYTE(s) (s & 0x00FF)
#define HIGH_BYTE(s) (s & 0xFF00) >> 8
#define HALF_SHORT(s) (UCHAR*)s + 1
#define SWAP_SHORT(s) (LOW_BYTE(s) << 8) | HIGH_BYTE(s)

using namespace std;

typedef map<UINT, struct CLASS_CODE_LOCAL*>::iterator LOCALS_ITERATOR;
typedef map<UINT, struct CLASS_CODE_LOCAL*> LOCALS_MAP;

struct DEX_HEADER 
{
    UCHAR    Magic[8];       /* includes version number */
    UINT    Checksum;        /* adler32 checksum */
    UCHAR   Signature[20];    /* SHA-1 hash */
    UINT    FileSize;       /* length of entire file */
    UINT    HeaderSize;     /* offset to start of next section */
    UINT    EndianTag;
    UINT    LinkSize;
    UINT    LinkOff;
    UINT    MapOff;
    UINT    StringIdsSize;
    UINT    StringIdsOff;
    UINT    TypeIdsSize;
    UINT    TypeIdsOff;
    UINT    ProtoIdsSize;
    UINT    ProtoIdsOff;
    UINT    FieldIdsSize;
    UINT    FieldIdsOff;
    UINT    MethodIdsSize;
    UINT    MethodIdsOff;
    UINT    ClassDefsSize;
    UINT    ClassDefsOff;
    UINT    DataSize;
    UINT    DataOff;
};

struct DEX_OPT_HEADER 
{
    UCHAR   Magic[8];           /* includes version number */
    UINT    DexOffset;          /* file offset of DEX header */
    UINT    DexLength;
    UINT    DepsOffset;         /* offset of optimized DEX dependency table */
    UINT    DepsLength;
    UINT    OptOffset;          /* file offset of optimized data tables */
    UINT    OptLength;
    UINT    Flags;              /* some info flags */
    UINT    Checksum;           /* adler32 checksum covering deps/opt */

    /* pad for 64-bit alignment if necessary */
};

struct DEX_FIELD_ID
{
    USHORT  ClassIndex;           /* index into typeIds list for defining class */
    USHORT  TypeIdex;            /* index into typeIds for field type */
    UINT    StringIndex;            /* index into stringIds for field name */
};

struct DEX_METHOD_ID
{
    USHORT  ClassIndex;           /* index into typeIds list for defining class */
    USHORT  PrototypeIndex;           /* index into protoIds for method prototype */
    UINT    StringIndex;            /* index into stringIds for method name */
};

struct DEX_PROTO_ID
{
    UINT    StringIndex;          /* index into stringIds for shorty descriptor */
    UINT    ReturnTypeIdx;      /* index into typeIds list for return type */
    UINT    ParametersOff;      /* file offset to type_list for parameter types */
};

struct DEX_CLASS_DATA_HEADER 
{
    UINT StaticFieldsSize;
    UINT InstanceFieldsSize;
    UINT DirectMethodsSize;
    UINT VirtualMethodsSize;
};

struct DEX_FIELD
{
    UINT FieldIdx;    /* index to a field_id_item */
    UINT AccessFlags;
};

struct DEX_METHOD 
{
    UINT MethodIdx;    /* index to a method_id_item */
    UINT AccessFlags;
    UINT CodeOff;      /* file offset to a code_item */
};

struct DEX_CLASS_DATA 
{
    DEX_CLASS_DATA_HEADER Header;
    DEX_FIELD*    StaticFields;
    DEX_FIELD*    InstanceFields;
    DEX_METHOD*   DirectMethods;
    DEX_METHOD*   VirtualMethods;
};

struct DEX_CLASS_DEF 
{
    UINT    ClassIdx;           /* index into typeIds for this class */
    UINT    AccessFlags;
    UINT    SuperclassIdx;      /* index into typeIds for superclass */
    UINT    InterfacesOff;      /* file offset to DexTypeList */
    UINT    SourceFileIdx;      /* index into stringIds for source file name */
    UINT    AnnotationsOff;     /* file offset to annotations_directory_item */
    UINT    ClassDataOff;       /* file offset to class_data_item */
    UINT    StaticValuesOff;    /* file offset to DexEncodedArray */
};

struct DEX_TYPE_ITEM    { USHORT    TypeIdx; };
struct DEX_LINK         { USHORT    Bleargh; };
struct DEX_STRING_ID    { UINT      StringDataOff; };
struct DEX_TYPE_ID      { UINT      StringIndex; };

struct DEX_FIELD_ANNOTATION
{
    UINT    FieldIdx;
    UINT    AnnotationsOff;
};

struct DEX_METHOD_ANNOTATION
{
    UINT    MethodIdx;
    UINT    AnnotationsOff;
};

struct DEX_PARAMETER_ANNOTATION
{
    UINT    MethodIdx;
    UINT    AnnotationsOff;
};


struct DEX_ANNOTATION_SET_REF_ITEM
{
    UINT AnnotationsOff;
};

struct DEX_ANNOTATION_SET_REF
{
    UINT Size;
    DEX_ANNOTATION_SET_REF_ITEM List[1];
};

struct DEX_ANNOTATION_OFF_ITEM
{
    UINT AnnotationOff;
};

struct DEX_ANNOTATION_ITEM
{
    UCHAR   Visibility;
    CHAR    Encoded[1];
};

struct DEX_ANNOTATION_SET_ITEM
{
    UINT Size;
    DEX_ANNOTATION_OFF_ITEM Entries[1];
};

struct DEX_ANNOTATIONS_DIRECTORY_ITEM
{
    UINT    ClassAnnotationsOff;
    UINT    FieldsSize;
    UINT    MethodsSize;
    UINT    ParametesrSize;
    UCHAR   Ptr[1];
};

struct DEX_TYPE_LIST
{
    UINT Size;
    DEX_TYPE_ITEM List[1];
};

struct DEX_CLASS_LOOKUP
{
    INT     Size;                       // total size, including "size"
    INT     NumEntries;                 // size of table[]; always power of 2
    struct {
        UINT    ClassDescriptorHash;    // class descriptor hash code
        INT     ClassDescriptorOffset;  // in bytes, from start of DEX
        INT     ClassDefOffset;         // in bytes, from start of DEX
    } Table[1];
};

struct DEX_STRING_ITEM
{
    UINT    StringSize;
    UCHAR*  Data;
};

struct DEX_TRY_ITEM
{
    UINT    StartAddress;
    USHORT  InstructionsSize;
    USHORT  HandlerOff;
};

struct DEX_FILE 
{
    //const DEX_OPT_HEADER* DexOptHeader;
    const DEX_HEADER*       DexHeader;
    const DEX_STRING_ID*    DexStringIds;
    const DEX_TYPE_ID*      DexTypeIds;
    const DEX_FIELD_ID*     DexFieldIds;
    const DEX_METHOD_ID*    DexMethodIds;
    const DEX_PROTO_ID*     DexProtoIds;
    const DEX_CLASS_DEF*    DexClassDefs;
    const DEX_LINK*         DexLinkData;

    const DEX_CLASS_LOOKUP* DexClassLookup;
    const void*             RegisterMapPool;
    const UCHAR*            BaseAddr;
    INT                     Overhead;
};

struct DEX_CODE 
{
    USHORT  RegistersSize;
    USHORT  InsSize;
    USHORT  OutsSize;
    USHORT  TriesSize;
    UINT    DebugInfoOff;       /* file offset to debug info stream */
    UINT    InstructionsSize;   /* size of the insns array, in u2 units */
    USHORT  Instructions[1];

    /* followed by optional u2 padding */
    /* followed by try_item[triesSize] */
    /* followed by uleb128 handlersSize */
    /* followed by catch_handler_item[handlersSize] */
};

#define DEX_MAGIC              "dex\n"
#define DEX_MAGIC_VERS         "036\0"
#define DEX_MAGIC_VERS_API_13  "035\0"
#define DEX_OPT_MAGIC          "dey\n"
#define DEX_OPT_MAGIC_VERS     "036\0"
#define DEX_DEP_MAGIC          "deps"

enum {
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED = 0x00020000,       // method (Dalvik only)

    ACC_CLASS_MASK = (ACC_PUBLIC| ACC_FINAL| ACC_INTERFACE| ACC_ABSTRACT| ACC_SYNTHETIC| ACC_ANNOTATION| ACC_ENUM),
    ACC_INNER_CLASS_MASK = (ACC_CLASS_MASK| ACC_PRIVATE| ACC_PROTECTED| ACC_STATIC),
    ACC_FIELD_MASK = (ACC_PUBLIC| ACC_PRIVATE| ACC_PROTECTED| ACC_STATIC| ACC_FINAL| ACC_VOLATILE| ACC_TRANSIENT| ACC_SYNTHETIC| ACC_ENUM),
    ACC_METHOD_MASK = (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                     | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
                     | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
                     | ACC_DECLARED_SYNCHRONIZED),
};

/* debug info opcodes and constants */
enum 
{
    DBG_END_SEQUENCE         = 0x00,
    DBG_ADVANCE_PC           = 0x01,
    DBG_ADVANCE_LINE         = 0x02,
    DBG_START_LOCAL          = 0x03,
    DBG_START_LOCAL_EXTENDED = 0x04,
    DBG_END_LOCAL            = 0x05,
    DBG_RESTART_LOCAL        = 0x06,
    DBG_SET_PROLOGUE_END     = 0x07,
    DBG_SET_EPILOGUE_BEGIN   = 0x08,
    DBG_SET_FILE             = 0x09,
    DBG_FIRST_SPECIAL        = 0x0a,
    DBG_LINE_BASE            = -4,
    DBG_LINE_RANGE           = 15,
};

enum
{
    VALUE_BYTE       = 0x00,
    VALUE_SHORT      = 0x02,
    VALUE_CHAR       = 0x03,
    VALUE_INT        = 0x04,
    VALUE_LONG       = 0x06,
    VALUE_FLOAT      = 0x10,
    VALUE_DOUBLE     = 0x11,
    VALUE_STRING     = 0x17,
    VALUE_TYPE       = 0x18,
    VALUE_FIELD      = 0x19,
    VALUE_METHOD     = 0x1a,
    VALUE_ENUM       = 0x1b,
    VALUE_ARRAY      = 0x1c,
    VALUE_ANNOTATION = 0x1d,
    VALUE_NULL       = 0x1e,
    VALUE_BOOLEAN    = 0x1f,
    VALUE_SENTINEL   = 0xff
};

#define kPackedSwitchSignature  0x0100
#define kSparseSwitchSignature  0x0200
#define kArrayDataSignature     0x0300



/* Dex Class Structures */

struct CLASS_ANNOTATION_ELEMENT
{
    UCHAR*  Name;
    UCHAR   ValueSize;
    UCHAR*  Value;
    UCHAR   ValueType;
};

struct CLASS_ANNOTATION
{
    UCHAR   Visibility;
    UCHAR*  Type;
    UINT    ElementsSize;
    CLASS_ANNOTATION_ELEMENT* Elements;
};

struct CLASS_FIELD 
{
    UCHAR* Name;
    UINT AccessFlags;
    UCHAR* Type;
    UCHAR* Value;
    CLASS_ANNOTATION* Annotations;
}; 

struct CLASS_CODE_DEBUG_POSITION
{
    UINT    Line;
    USHORT  Offset;
};

struct CLASS_CODE_DEBUG_INFO
{
    UINT PositionsSize;
    CLASS_CODE_DEBUG_POSITION** Positions;
};

struct CLASS_CODE_CATCH_TYPE_PAIR
{
    UCHAR* Type;
    UINT TypeIndex;
    UINT Address;
};

struct CLASS_CODE_CATCH_HANDLER
{
    INT TypeHandlersSize;
    CLASS_CODE_CATCH_TYPE_PAIR *TypeHandlers;
    UINT CatchAllAddress;
};

struct CLASS_CODE_TRY
{
    UINT InstructionsStart;
    UINT InstructionsEnd;
    CLASS_CODE_CATCH_HANDLER* CatchHandler;
};

struct CLASS_CODE_INSTRUCTION
{
    UCHAR*  Opcode;
    UCHAR   OpcodeSig;
    UINT    Offset;
    UCHAR*  Format;
    USHORT  BufferSize;
    USHORT* Buffer;
    CHAR*  Decoded;
    CHAR*  Decompiled;

    UINT    vA;
    UINT    vB;
    UINT64  vB_wide;        /* for OP_FORMAT_51l */
    UINT    vC;
    UINT    vArg[5];
};

struct CLASS_CODE_LOCAL
{
    CHAR*   Name;
    CHAR*   Type;
};

struct CLASS_CODE_REGISTER
{
    CHAR*   Name;
    CHAR*   Value;
    CHAR*   Type;
    USHORT  StartAddress;
    USHORT  EndAddress;
    CHAR*   Signature;
    BOOL    Local;
    BOOL    Initialized;
    CLASS_CODE_REGISTER* Next;
};

struct CLASS_CODE
{
    USHORT  RegistersSize;
    CLASS_CODE_REGISTER** Registers;
    
    USHORT  InsSize;
    USHORT  OutsSize;  
    UINT    InstBufferSize; 
    UINT    Offset;

    CLASS_CODE_DEBUG_INFO DebugInfo;

    UINT CatchHandlersSize;
    CLASS_CODE_CATCH_HANDLER *CatchHandlers;

    CLASS_CODE_TRY *Tries;
    USHORT  TriesSize;

    CLASS_CODE_INSTRUCTION   **Instructions;
    UINT    InstructionsSize;

    map<UINT, CLASS_CODE_LOCAL*>* Locals;
};

struct CLASS_METHOD 
{
    UCHAR* Name;
    UINT AccessFlags;
    UCHAR* Type;
    UCHAR* ProtoType;
    CLASS_CODE  *CodeArea;
    CLASS_ANNOTATION* Annotations;
};

struct CLASS_DATA
{
    UINT StaticFieldsSize;
    UINT InstanceFieldsSize;
    UINT DirectMethodsSize;
    UINT VirtualMethodsSize;
    
    UINT InterfacesSize;
    UCHAR** Interfaces;

    CLASS_FIELD  *StaticFields, *InstanceFields;
    CLASS_METHOD  *DirectMethods, *VirtualMethods;

    UINT    AnnotationsSize;
    CLASS_ANNOTATION * Annotations;
};

struct DEX_CLASS_STRUCTURE
{
    UCHAR*  Descriptor;
    UINT    AccessFlags;
    UCHAR*  SuperClass;
    UCHAR*  SourceFile;

    CLASS_DATA * ClassData;

    DEX_CLASS_DEF* Ref;
};


static const CHAR* OpcodesFormatStrings[32] = 
{
    "?",
    "op",
    "op vA, vB",
    "op vA, #+B",
    "op vAA",
    "op +AA",
    "op AA, thing@BBBB",
    "op +AAAA",
    "op vAA, vBBBB",
    "op vAA, +BBBB",
    "op vAA, #+BBBB",
    "op vAA, #+BBBB00000[00000000]",
    "op vAA, thing@BBBB",
    "op vAA, vBB, vCC",
    "op vAA, vBB, #+CC",
    "op vA, vB, +CCCC",
    "op vA, vB, #+CCCC",
    "op vA, vB, thing@CCCC",
    "[opt] op vA, vB, field offset CCCC",
    "op vAAAA, vBBBB",
    "op +AAAAAAAA",
    "op vAA, +BBBBBBBB",
    "op vAA, #+BBBBBBBB",
    "op vAA, thing@BBBBBBBB",
    "op {vC, vD, vE, vF, vG}, thing@BBBB (B: count, A: vG)",
    "[opt] invoke-virtual+super",
    "[opt] invoke-interface",
    "op {vCCCC .. v(CCCC+AA-1)}, meth@BBBB",
    "[opt] invoke-virtual+super/range",
    "[opt] invoke-interface/range",
    "[opt] inline invoke",
    "op vAA, #+BBBBBBBBBBBBBBBB",
};

static const CHAR* OpcodesStrings[256] =
{
    "nop",
    "move",
    "move/from16",
    "move/16",
    "move-wide",
    "move-wide/from16",
    "move-wide/16",
    "move-object",
    "move-object/from16",
    "move-object/16",
    
    "move-result",
    "move-result-wide",
    "move-result-object",
    "move-exception",

    "return-void",
    "return",
    "return-wide",
    "return-object",

    "const/4",
    "const/16",
    "const",
    "const/high16",
    "const-wide/16",
    "const-wide/32",
    "const-wide",
    "const-wide/high16",
    "const-string",
    "const-string/jumbo",
    "const-class",

    "monitor-enter",
    "monitor-exit",
    "check-cast",
    "instance-of",
    "array-length",
    "new-instance",
    "new-array",
    "filled-new-array",
    "filled-new-array/range",
    "fill-array-data",
    "throw",
    "goto",
    "goto/16",
    "goto/32",
    "packed-switch",
    "sparse-switch",

    "cmpl-float",
    "cmpg-float",
    "cmpl-double",
    "cmpg-double",
    "cmp-long",

    "if-eq",
    "if-ne",
    "if-lt",
    "if-ge",
    "if-gt",
    "if-le",

    "if-eqz",
    "if-nez",
    "if-ltz",
    "if-gez",
    "if-gtz",
    "if-lez",

    "?",
    "?",
    "?",
    "?",
    "?",
    "?",

    "aget",
    "aget-wide",
    "aget-object",
    "aget-boolean",
    "aget-byte",
    "aget-char",
    "aget-short",
    "aput",
    "aput-wide",
    "aput-object",
    "aput-boolean",
    "aput-byte",
    "aput-char",
    "aput-short",

    "iget",
    "iget-wide",
    "iget-object",
    "iget-boolean",
    "iget-byte",
    "iget-char",
    "iget-short",
    "iput",
    "iput-wide",
    "iput-object",
    "iput-boolean",
    "iput-byte",
    "iput-char",
    "iput-short",

    "sget",
    "sget-wide",
    "sget-object",
    "sget-boolean",
    "sget-byte",
    "sget-char",
    "sget-short",
    "sput",
    "sput-wide",
    "sput-object",
    "sput-boolean",
    "sput-byte",
    "sput-char",
    "sput-short",

    "invoke-virtual",
    "invoke-super",
    "invoke-direct",
    "invoke-static",
    "invoke-interface",

    "?",

    "invoke-virtual/range",
    "invoke-super/range",
    "invoke-direct/range",
    "invoke-static/range",
    "invoke-interface/range",

    "?",
    "?",

    "neg-int",
    "not-int",
    "neg-long",
    "not-long",
    "neg-float",
    "neg-double",
    "int-to-long",
    "int-to-float",
    "int-to-double",
    "long-to-int",
    "long-to-float",
    "long-to-double",
    "float-to-int",
    "float-to-long",
    "float-to-double",
    "double-to-int",
    "double-to-long",
    "double-to-float",
    "int-to-byte",
    "int-to-char",
    "int-to-short",

    "add-int",
    "sub-int",
    "mul-int",
    "div-int",
    "rem-int",
    "and-int",
    "or-int",
    "xor-int",
    "shl-int",
    "shr-int",
    "ushr-int",
    "add-long",
    "sub-long",
    "mul-long",
    "div-long",
    "rem-long",
    "and-long",
    "or-long",
    "xor-long",
    "shl-long",
    "shr-long",
    "ushr-long",
    "add-float",
    "sub-float",
    "mul-float",
    "div-float",
    "rem-float",
    "add-double",
    "sub-double",
    "mul-double",
    "div-double",
    "rem-double",

    "add-int/2addr",
    "sub-int/2addr",
    "mul-int/2addr",
    "div-int/2addr",
    "rem-int/2addr",
    "and-int/2addr",
    "or-int/2addr",
    "xor-int/2addr",
    "shl-int/2addr",
    "shr-int/2addr",
    "ushr-int/2addr",
    "add-long/2addr",
    "sub-long/2addr",
    "mul-long/2addr",
    "div-long/2addr",
    "rem-long/2addr",
    "and-long/2addr",
    "or-long/2addr",
    "xor-long/2addr",
    "shl-long/2addr",
    "shr-long/2addr",
    "ushr-long/2addr",
    "add-float/2addr",
    "sub-float/2addr",
    "mul-float/2addr",
    "div-float/2addr",
    "rem-float/2addr",
    "add-double/2addr",
    "sub-double/2addr",
    "mul-double/2addr",
    "div-double/2addr",
    "rem-double/2addr",

    "add-int/lit16",
    "rsub-int",
    "mul-int/lit16",
    "div-int/lit16",
    "rem-int/lit16",
    "and-int/lit16",
    "or-int/lit16",
    "xor-int/lit16",

    "add-int/lit8",
    "rsub-int/lit8",
    "mul-int/lit8",
    "div-int/lit8",
    "rem-int/lit8",
    "and-int/lit8",
    "or-int/lit8",
    "xor-int/lit8",
    "shl-int/lit8",
    "shr-int/lit8",
    "ushr-int/lit8",

    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
};

typedef enum DEX_OP_CODES 
{
    OP_NOP                          = 0x00,

    OP_MOVE                         = 0x01,
    OP_MOVE_FROM16                  = 0x02,
    OP_MOVE_16                      = 0x03,
    OP_MOVE_WIDE                    = 0x04,
    OP_MOVE_WIDE_FROM16             = 0x05,
    OP_MOVE_WIDE_16                 = 0x06,
    OP_MOVE_OBJECT                  = 0x07,
    OP_MOVE_OBJECT_FROM16           = 0x08,
    OP_MOVE_OBJECT_16               = 0x09,

    OP_MOVE_RESULT                  = 0x0a,
    OP_MOVE_RESULT_WIDE             = 0x0b,
    OP_MOVE_RESULT_OBJECT           = 0x0c,
    OP_MOVE_EXCEPTION               = 0x0d,

    OP_RETURN_VOID                  = 0x0e,
    OP_RETURN                       = 0x0f,
    OP_RETURN_WIDE                  = 0x10,
    OP_RETURN_OBJECT                = 0x11,

    OP_CONST_4                      = 0x12,
    OP_CONST_16                     = 0x13,
    OP_CONST                        = 0x14,
    OP_CONST_HIGH16                 = 0x15,
    OP_CONST_WIDE_16                = 0x16,
    OP_CONST_WIDE_32                = 0x17,
    OP_CONST_WIDE                   = 0x18,
    OP_CONST_WIDE_HIGH16            = 0x19,
    OP_CONST_STRING                 = 0x1a,
    OP_CONST_STRING_JUMBO           = 0x1b,
    OP_CONST_CLASS                  = 0x1c,

    OP_MONITOR_ENTER                = 0x1d,
    OP_MONITOR_EXIT                 = 0x1e,

    OP_CHECK_CAST                   = 0x1f,
    OP_INSTANCE_OF                  = 0x20,

    OP_ARRAY_LENGTH                 = 0x21,

    OP_NEW_INSTANCE                 = 0x22,
    OP_NEW_ARRAY                    = 0x23,
    
    OP_FILLED_NEW_ARRAY             = 0x24,
    OP_FILLED_NEW_ARRAY_RANGE       = 0x25,
    OP_FILL_ARRAY_DATA              = 0x26,
    
    OP_THROW                        = 0x27,
    OP_GOTO                         = 0x28,
    OP_GOTO_16                      = 0x29,
    OP_GOTO_32                      = 0x2a,
    OP_PACKED_SWITCH                = 0x2b,
    OP_SPARSE_SWITCH                = 0x2c,
    
    OP_CMPL_FLOAT                   = 0x2d,
    OP_CMPG_FLOAT                   = 0x2e,
    OP_CMPL_DOUBLE                  = 0x2f,
    OP_CMPG_DOUBLE                  = 0x30,
    OP_CMP_LONG                     = 0x31,

    OP_IF_EQ                        = 0x32,
    OP_IF_NE                        = 0x33,
    OP_IF_LT                        = 0x34,
    OP_IF_GE                        = 0x35,
    OP_IF_GT                        = 0x36,
    OP_IF_LE                        = 0x37,
    OP_IF_EQZ                       = 0x38,
    OP_IF_NEZ                       = 0x39,
    OP_IF_LTZ                       = 0x3a,
    OP_IF_GEZ                       = 0x3b,
    OP_IF_GTZ                       = 0x3c,
    OP_IF_LEZ                       = 0x3d,

    OP_UNUSED_3E                    = 0x3e,
    OP_UNUSED_3F                    = 0x3f,
    OP_UNUSED_40                    = 0x40,
    OP_UNUSED_41                    = 0x41,
    OP_UNUSED_42                    = 0x42,
    OP_UNUSED_43                    = 0x43,
    
    OP_AGET                         = 0x44,
    OP_AGET_WIDE                    = 0x45,
    OP_AGET_OBJECT                  = 0x46,
    OP_AGET_BOOLEAN                 = 0x47,
    OP_AGET_BYTE                    = 0x48,
    OP_AGET_CHAR                    = 0x49,
    OP_AGET_SHORT                   = 0x4a,
    OP_APUT                         = 0x4b,
    OP_APUT_WIDE                    = 0x4c,
    OP_APUT_OBJECT                  = 0x4d,
    OP_APUT_BOOLEAN                 = 0x4e,
    OP_APUT_BYTE                    = 0x4f,
    OP_APUT_CHAR                    = 0x50,
    OP_APUT_SHORT                   = 0x51,

    OP_IGET                         = 0x52,
    OP_IGET_WIDE                    = 0x53,
    OP_IGET_OBJECT                  = 0x54,
    OP_IGET_BOOLEAN                 = 0x55,
    OP_IGET_BYTE                    = 0x56,
    OP_IGET_CHAR                    = 0x57,
    OP_IGET_SHORT                   = 0x58,
    OP_IPUT                         = 0x59,
    OP_IPUT_WIDE                    = 0x5a,
    OP_IPUT_OBJECT                  = 0x5b,
    OP_IPUT_BOOLEAN                 = 0x5c,
    OP_IPUT_BYTE                    = 0x5d,
    OP_IPUT_CHAR                    = 0x5e,
    OP_IPUT_SHORT                   = 0x5f,

    OP_SGET                         = 0x60,
    OP_SGET_WIDE                    = 0x61,
    OP_SGET_OBJECT                  = 0x62,
    OP_SGET_BOOLEAN                 = 0x63,
    OP_SGET_BYTE                    = 0x64,
    OP_SGET_CHAR                    = 0x65,
    OP_SGET_SHORT                   = 0x66,
    OP_SPUT                         = 0x67,
    OP_SPUT_WIDE                    = 0x68,
    OP_SPUT_OBJECT                  = 0x69,
    OP_SPUT_BOOLEAN                 = 0x6a,
    OP_SPUT_BYTE                    = 0x6b,
    OP_SPUT_CHAR                    = 0x6c,
    OP_SPUT_SHORT                   = 0x6d,

    OP_INVOKE_VIRTUAL               = 0x6e,
    OP_INVOKE_SUPER                 = 0x6f,
    OP_INVOKE_DIRECT                = 0x70,
    OP_INVOKE_STATIC                = 0x71,
    OP_INVOKE_INTERFACE             = 0x72,

    OP_UNUSED_73                    = 0x73,
    
    OP_INVOKE_VIRTUAL_RANGE         = 0x74,
    OP_INVOKE_SUPER_RANGE           = 0x75,
    OP_INVOKE_DIRECT_RANGE          = 0x76,
    OP_INVOKE_STATIC_RANGE          = 0x77,
    OP_INVOKE_INTERFACE_RANGE       = 0x78,

    OP_UNUSED_79                    = 0x79,
    OP_UNUSED_7A                    = 0x7a,

    OP_NEG_INT                      = 0x7b,
    OP_NOT_INT                      = 0x7c,
    OP_NEG_LONG                     = 0x7d,
    OP_NOT_LONG                     = 0x7e,
    OP_NEG_FLOAT                    = 0x7f,
    OP_NEG_DOUBLE                   = 0x80,
    OP_INT_TO_LONG                  = 0x81,
    OP_INT_TO_FLOAT                 = 0x82,
    OP_INT_TO_DOUBLE                = 0x83,
    OP_LONG_TO_INT                  = 0x84,
    OP_LONG_TO_FLOAT                = 0x85,
    OP_LONG_TO_DOUBLE               = 0x86,
    OP_FLOAT_TO_INT                 = 0x87,
    OP_FLOAT_TO_LONG                = 0x88,
    OP_FLOAT_TO_DOUBLE              = 0x89,
    OP_DOUBLE_TO_INT                = 0x8a,
    OP_DOUBLE_TO_LONG               = 0x8b,
    OP_DOUBLE_TO_FLOAT              = 0x8c,
    OP_INT_TO_BYTE                  = 0x8d,
    OP_INT_TO_CHAR                  = 0x8e,
    OP_INT_TO_SHORT                 = 0x8f,

    OP_ADD_INT                      = 0x90,
    OP_SUB_INT                      = 0x91,
    OP_MUL_INT                      = 0x92,
    OP_DIV_INT                      = 0x93,
    OP_REM_INT                      = 0x94,
    OP_AND_INT                      = 0x95,
    OP_OR_INT                       = 0x96,
    OP_XOR_INT                      = 0x97,
    OP_SHL_INT                      = 0x98,
    OP_SHR_INT                      = 0x99,
    OP_USHR_INT                     = 0x9a,

    OP_ADD_LONG                     = 0x9b,
    OP_SUB_LONG                     = 0x9c,
    OP_MUL_LONG                     = 0x9d,
    OP_DIV_LONG                     = 0x9e,
    OP_REM_LONG                     = 0x9f,
    OP_AND_LONG                     = 0xa0,
    OP_OR_LONG                      = 0xa1,
    OP_XOR_LONG                     = 0xa2,
    OP_SHL_LONG                     = 0xa3,
    OP_SHR_LONG                     = 0xa4,
    OP_USHR_LONG                    = 0xa5,

    OP_ADD_FLOAT                    = 0xa6,
    OP_SUB_FLOAT                    = 0xa7,
    OP_MUL_FLOAT                    = 0xa8,
    OP_DIV_FLOAT                    = 0xa9,
    OP_REM_FLOAT                    = 0xaa,
    OP_ADD_DOUBLE                   = 0xab,
    OP_SUB_DOUBLE                   = 0xac,
    OP_MUL_DOUBLE                   = 0xad,
    OP_DIV_DOUBLE                   = 0xae,
    OP_REM_DOUBLE                   = 0xaf,

    OP_ADD_INT_2ADDR                = 0xb0,
    OP_SUB_INT_2ADDR                = 0xb1,
    OP_MUL_INT_2ADDR                = 0xb2,
    OP_DIV_INT_2ADDR                = 0xb3,
    OP_REM_INT_2ADDR                = 0xb4,
    OP_AND_INT_2ADDR                = 0xb5,
    OP_OR_INT_2ADDR                 = 0xb6,
    OP_XOR_INT_2ADDR                = 0xb7,
    OP_SHL_INT_2ADDR                = 0xb8,
    OP_SHR_INT_2ADDR                = 0xb9,
    OP_USHR_INT_2ADDR               = 0xba,

    OP_ADD_LONG_2ADDR               = 0xbb,
    OP_SUB_LONG_2ADDR               = 0xbc,
    OP_MUL_LONG_2ADDR               = 0xbd,
    OP_DIV_LONG_2ADDR               = 0xbe,
    OP_REM_LONG_2ADDR               = 0xbf,
    OP_AND_LONG_2ADDR               = 0xc0,
    OP_OR_LONG_2ADDR                = 0xc1,
    OP_XOR_LONG_2ADDR               = 0xc2,
    OP_SHL_LONG_2ADDR               = 0xc3,
    OP_SHR_LONG_2ADDR               = 0xc4,
    OP_USHR_LONG_2ADDR              = 0xc5,

    OP_ADD_FLOAT_2ADDR              = 0xc6,
    OP_SUB_FLOAT_2ADDR              = 0xc7,
    OP_MUL_FLOAT_2ADDR              = 0xc8,
    OP_DIV_FLOAT_2ADDR              = 0xc9,
    OP_REM_FLOAT_2ADDR              = 0xca,
    OP_ADD_DOUBLE_2ADDR             = 0xcb,
    OP_SUB_DOUBLE_2ADDR             = 0xcc,
    OP_MUL_DOUBLE_2ADDR             = 0xcd,
    OP_DIV_DOUBLE_2ADDR             = 0xce,
    OP_REM_DOUBLE_2ADDR             = 0xcf,

    OP_ADD_INT_LIT16                = 0xd0,
    OP_RSUB_INT                     = 0xd1, /* no _LIT16 suffix for this */
    OP_MUL_INT_LIT16                = 0xd2,
    OP_DIV_INT_LIT16                = 0xd3,
    OP_REM_INT_LIT16                = 0xd4,
    OP_AND_INT_LIT16                = 0xd5,
    OP_OR_INT_LIT16                 = 0xd6,
    OP_XOR_INT_LIT16                = 0xd7,

    OP_ADD_INT_LIT8                 = 0xd8,
    OP_RSUB_INT_LIT8                = 0xd9,
    OP_MUL_INT_LIT8                 = 0xda,
    OP_DIV_INT_LIT8                 = 0xdb,
    OP_REM_INT_LIT8                 = 0xdc,
    OP_AND_INT_LIT8                 = 0xdd,
    OP_OR_INT_LIT8                  = 0xde,
    OP_XOR_INT_LIT8                 = 0xdf,
    OP_SHL_INT_LIT8                 = 0xe0,
    OP_SHR_INT_LIT8                 = 0xe1,
    OP_USHR_INT_LIT8                = 0xe2,

    OP_UNUSED_E3                    = 0xe3,
    OP_UNUSED_E4                    = 0xe4,
    OP_UNUSED_E5                    = 0xe5,
    OP_UNUSED_E6                    = 0xe6,
    OP_UNUSED_E7                    = 0xe7,
    OP_UNUSED_E8                    = 0xe8,
    OP_UNUSED_E9                    = 0xe9,
    OP_UNUSED_EA                    = 0xea,
    OP_UNUSED_EB                    = 0xeb,
    OP_UNUSED_EC                    = 0xec,

    /* optimizer output -- these are never generated by "dx" */
    OP_THROW_VERIFICATION_ERROR     = 0xed,
    OP_EXECUTE_INLINE               = 0xee,
    OP_UNUSED_EF                    = 0xef, /* OP_EXECUTE_INLINE_RANGE? */

    OP_INVOKE_DIRECT_EMPTY          = 0xf0,
    OP_UNUSED_F1                    = 0xf1, /* OP_INVOKE_DIRECT_EMPTY_RANGE? */
    OP_IGET_QUICK                   = 0xf2,
    OP_IGET_WIDE_QUICK              = 0xf3,
    OP_IGET_OBJECT_QUICK            = 0xf4,
    OP_IPUT_QUICK                   = 0xf5,
    OP_IPUT_WIDE_QUICK              = 0xf6,
    OP_IPUT_OBJECT_QUICK            = 0xf7,

    OP_INVOKE_VIRTUAL_QUICK         = 0xf8,
    OP_INVOKE_VIRTUAL_QUICK_RANGE   = 0xf9,
    OP_INVOKE_SUPER_QUICK           = 0xfa,
    OP_INVOKE_SUPER_QUICK_RANGE     = 0xfb,
    OP_UNUSED_FC                    = 0xfc, /* OP_INVOKE_DIRECT_QUICK? */
    OP_UNUSED_FD                    = 0xfd, /* OP_INVOKE_DIRECT_QUICK_RANGE? */
    OP_UNUSED_FE                    = 0xfe, /* OP_INVOKE_INTERFACE_QUICK? */
    OP_UNUSED_FF                    = 0xff, /* OP_INVOKE_INTERFACE_QUICK_RANGE*/
};

enum DEX_OP_CODES_FLAGS 
{
    OP_FLAG_CAN_BRANCH    = 1,        // conditional or unconditional branch
    OP_FLAG_CAN_CONTINUE  = 1 << 1,   // flow can continue to next statement
    OP_FLAG_CAN_SWITCH    = 1 << 2,   // switch statement
    OP_FLAG_CAN_THROW     = 1 << 3,   // could cause an exception to be thrown
    OP_FLAG_CAN_RETURN    = 1 << 4,   // returns, no additional statements
    OP_FLAG_INVOKE        = 1 << 5,   // a flavor of invoke
    OP_FLAG_UNCONDITIONAL = 1 << 6,   // unconditional branch
};

enum DEX_OP_CODES_FORMAT 
{
    OP_FORMAT_UNKNOWN = 0,
    OP_FORMAT_10x,        // op
    OP_FORMAT_12x,        // op vA, vB
    OP_FORMAT_11n,        // op vA, #+B
    OP_FORMAT_11x,        // op vAA
    OP_FORMAT_10t,        // op +AA
    OP_FORMAT_20bc,       // op AA, thing@BBBB
    OP_FORMAT_20t,        // op +AAAA
    OP_FORMAT_22x,        // op vAA, vBBBB
    OP_FORMAT_21t,        // op vAA, +BBBB
    OP_FORMAT_21s,        // op vAA, #+BBBB
    OP_FORMAT_21h,        // op vAA, #+BBBB00000[00000000]
    OP_FORMAT_21c,        // op vAA, thing@BBBB
    OP_FORMAT_23x,        // op vAA, vBB, vCC
    OP_FORMAT_22b,        // op vAA, vBB, #+CC
    OP_FORMAT_22t,        // op vA, vB, +CCCC
    OP_FORMAT_22s,        // op vA, vB, #+CCCC
    OP_FORMAT_22c,        // op vA, vB, thing@CCCC
    OP_FORMAT_22cs,       // [opt] op vA, vB, field offset CCCC
    OP_FORMAT_32x,        // op vAAAA, vBBBB
    OP_FORMAT_30t,        // op +AAAAAAAA
    OP_FORMAT_31t,        // op vAA, +BBBBBBBB
    OP_FORMAT_31i,        // op vAA, #+BBBBBBBB
    OP_FORMAT_31c,        // op vAA, thing@BBBBBBBB
    OP_FORMAT_35c,        // op {vC, vD, vE, vF, vG}, thing@BBBB (B: count, A: vG)
    OP_FORMAT_35ms,       // [opt] invoke-virtual+super
    OP_FORMAT_35fs,       // [opt] invoke-interface
    OP_FORMAT_3rc,        // op {vCCCC .. v(CCCC+AA-1)}, meth@BBBB
    OP_FORMAT_3rms,       // [opt] invoke-virtual+super/range
    OP_FORMAT_3rfs,       // [opt] invoke-interface/range
    OP_FORMAT_3inline,    // [opt] inline invoke
    OP_FORMAT_51l,        // op vAA, #+BBBBBBBBBBBBBBBB
};

struct DEX_FIELD_STATIC_VALUE
{

};

class DLLEXPORT cDexFile: public cFile
{
public:
    cDexFile(CHAR* Filename);
    cDexFile(CHAR* Buffer, DWORD Size);
    ~cDexFile();

    BOOL    isReady;

    UCHAR   DexVersion[4];
    UINT    nStringIDs,
            nFieldIDs,
            nTypeIDs,
            nMethodIDs,
            nPrototypeIDs,
            nClassDefinitions,
            nStringItems;

    DEX_HEADER*     DexHeader;
    DEX_STRING_ID*  DexStringIds;
    DEX_TYPE_ID*    DexTypeIds;
    DEX_FIELD_ID*   DexFieldIds;
    DEX_FIELD_STATIC_VALUE* DexFieldsValues;
    DEX_METHOD_ID*  DexMethodIds;
    DEX_PROTO_ID*   DexProtoIds;
    DEX_CLASS_DEF*  DexClassDefs;
    DEX_LINK*       DexLinkData;
    DEX_CLASS_DATA* DexClassData;

    DEX_STRING_ITEM* StringItems;

    UINT nClasses;
    DEX_CLASS_STRUCTURE* DexClasses;

   

    void DumpClassInfo(UINT ClassIndex, DEX_CLASS_STRUCTURE* Class);
    void DumpClassDataInfo(UINT ClassIndex, DEX_CLASS_STRUCTURE* Class, UCHAR** Buffer);

    void DumpFieldsValues(UINT Offset, CLASS_DATA* ClassData);

    void DumpFieldByIndex(UINT FieldIndex, CLASS_FIELD* Field, UCHAR** Buffer);
    void DumpInterfaceByIndex(UINT ClassIndex, UINT InterfaceIndex, UCHAR** Interface);
    
    void InsertDebugPosition(CLASS_CODE* CodeArea, UINT Line, USHORT Offset);

    void DumpMethodTryItems(CLASS_CODE* CodeArea, DEX_CODE* CodeAreaDef);
    void DumpMethodTryItemsInfo(CLASS_CODE_TRY* TryItem, DEX_TRY_ITEM* TryItemInfo, CLASS_CODE_CATCH_HANDLER** CatchHandlers);
    void DumpMethodCatchHandlers(CLASS_CODE* CodeArea, UCHAR** Buffer);
    void DumpMethodDebugInfo(CLASS_METHOD* Method, CLASS_CODE_DEBUG_INFO* DebugInfo, UCHAR** Buffer);
    void DumpMethodLocals(CLASS_METHOD* Method, UCHAR** Buffer);
    void DumpMethodCodeInfo(CLASS_CODE* CodeArea, DEX_CODE* CodeAreaDef);
    void DumpMethodCode(DEX_CODE* CodeAreaDef, CLASS_METHOD* Method);
    void DumpMethodById(UINT MethodIndex, CLASS_METHOD* Method, UCHAR** Buffer);
    void DumpMethodInstructions(CLASS_CODE* CodeArea, DEX_CODE* CodeAreaDef);
    void DumpMethodParameters(UINT MethodIndex, CLASS_METHOD* Method);
    
    void AllocateClassData(UINT ClassIndex, DEX_CLASS_STRUCTURE* Class);

    void DumpAnnotations(DEX_CLASS_STRUCTURE* DexClass, UINT Offset);
    void DumpAnnotationElementValue(CLASS_ANNOTATION_ELEMENT* Element, UCHAR** Ptr);

    CLASS_CODE_INSTRUCTION* DecodeOpcode(USHORT* Opcode);

private:
    BOOL    DumpDex();
    INT     ReadUnsignedLeb128(const UCHAR** pStream);
    INT     ReadSignedLeb128(const UCHAR** pStream);
    UINT    ULEB128toUINT(UCHAR *data);
    UCHAR*  ULEB128toUCHAR(UCHAR *data, UINT *v);

    BOOL    ValidChecksum();
    DEX_CODE* DexCode;

    CHAR*   OpcodesWidths;
    CHAR*   OpcodesFlags;
    CHAR*   OpcodesFormat;

    void    CreateOpcodesWidthsTable();
    void    CreateOpcodesFlagsTable();
    void    CreateOpcodesFormatTable();

    UCHAR*  TempBuffer;

    ULONG   OpcodeCounter;

    CLASS_CODE_REGISTER* AddToRegisters(UINT Index, CLASS_CODE_REGISTER** Registers);
    CLASS_CODE_REGISTER* GetUnendedRegister(UINT Index, CLASS_CODE_REGISTER** Registers);
    CLASS_CODE_REGISTER* RestartRegister(UINT Index, CLASS_CODE_REGISTER** Registers);
};


