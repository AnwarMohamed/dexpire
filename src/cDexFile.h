#pragma once
#include "dexpire.h"
#include "cFile.h"

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
    USHORT* Instructions;
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

struct DEX_CLASS_STRUCTURE
{
	UCHAR*	Descriptor;
	UINT	AccessFlags;
	UCHAR*	SuperClass;
	UCHAR*	SourceFile;

	struct CLASS_DATA
	{
		UINT StaticFieldsSize;
		UINT InstanceFieldsSize;
		UINT DirectMethodsSize;
		UINT VirtualMethodsSize;
		UINT InterfacesSize;

		UCHAR** Interfaces;

		struct CLASS_FIELD 
		{
			UCHAR* Name;
			UINT AccessFlags;
			UCHAR* Type;
		}	*StaticFields, 
			*InstanceFields;

		struct CLASS_METHOD 
		{
			UCHAR* Name;
			UINT AccessFlags;
			UCHAR* Type;
			UCHAR* ProtoType;

			struct CLASS_CODE
			{
				USHORT  RegistersSize;
				USHORT  InsSize;
				USHORT  OutsSize;
				USHORT  TriesSize;
				UINT    DebugInfoOff;   
				UINT    InstructionsSize;  

				struct CLASS_CODE_TRY
				{
				} *Tries;

				struct CLASS_CODE_INSTRUCTION
				{
					UINT n;
				}	*Instructions;
			}	*CodeArea;

		}	*DirectMethods, 
			*VirtualMethods;

	}*	ClassData;
};




class DLLEXPORT cDexFile: public cFile
{
public:
    cDexFile(CHAR* Filename);
    ~cDexFile(void);

	BOOL	isReady;

	UCHAR	DexVersion[4];
	UINT	nStringIDs,
			nFieldIDs,
			nTypeIDs,
			nMethodIDs,
			nPrototypeIDs,
			nClassDefinitions,
			nStringItems;

	DEX_HEADER*		DexHeader;
	DEX_STRING_ID*	DexStringIds;
	DEX_TYPE_ID*	DexTypeIds;
	DEX_FIELD_ID*	DexFieldIds;
	DEX_METHOD_ID*	DexMethodIds;
	DEX_PROTO_ID*	DexProtoIds;
	DEX_CLASS_DEF*	DexClassDefs;
	DEX_LINK*		DexLinkData;
	DEX_CLASS_DATA*	DexClassData;

	DEX_STRING_ITEM* StringItems;

	UINT nClasses;
	DEX_CLASS_STRUCTURE* DexClasses;

	CHAR*	GetAccessMask(UINT Type, UINT AccessFlag);

private:
    BOOL	DumpDex();
	INT		ReadUnsignedLeb128(const UCHAR** pStream);
	UINT	ULEB128toUINT(UCHAR *data);
	UCHAR*	ULEB128toUCHAR(UCHAR *data, UINT *v);

	BOOL	ValidChecksum();
	DEX_CODE* DexCode;
};

