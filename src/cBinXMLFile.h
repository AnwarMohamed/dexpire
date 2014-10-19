/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#pragma once
#include "cFile.h"

enum {
    RES_NULL_TYPE               = 0x0000,
    RES_STRING_POOL_TYPE        = 0x0001,
    RES_TABLE_TYPE              = 0x0002,
    RES_XML_TYPE                = 0x0003,

    // Chunk types in RES_XML_TYPE
    RES_XML_FIRST_CHUNK_TYPE    = 0x0100,
    RES_XML_START_NAMESPACE_TYPE= 0x0100,
    RES_XML_END_NAMESPACE_TYPE  = 0x0101,
    RES_XML_START_ELEMENT_TYPE  = 0x0102,
    RES_XML_END_ELEMENT_TYPE    = 0x0103,
    RES_XML_CDATA_TYPE          = 0x0104,
    RES_XML_LAST_CHUNK_TYPE     = 0x017f,
    // This contains a uint32_t array mapping strings in the string
    // pool back to resource identifiers.  It is optional.
    RES_XML_RESOURCE_MAP_TYPE   = 0x0180,

    // Chunk types in RES_TABLE_TYPE
    RES_TABLE_PACKAGE_TYPE      = 0x0200,
    RES_TABLE_TYPE_TYPE         = 0x0201,
    RES_TABLE_TYPE_SPEC_TYPE    = 0x0202
};

STRUCT RES_CHUNK_HEADER
{
    // Type identifier for this chunk.  The meaning of this value depends
    // on the containing chunk.
    USHORT Type;

    // Size of the chunk header (in bytes).  Adding this value to
    // the address of the chunk allows you to find its associated data
    // (if any).
    USHORT HeaderSize;

    // Total size of this chunk (in bytes).  This is the chunkSize plus
    // the size of any data associated with the chunk.  Adding this value
    // to the chunk allows you to completely skip its contents (including
    // any child chunks).  If this value is the same as chunkSize, there is
    // no data associated with the chunk.
    UINT Size;
};

STRUCT RES_STRING_POOL_HEADER
{
    UINT StringCount;
    UINT StyleCount;
    UINT Flags;
    UINT StringStart;
    UINT StylesStart;
};

STRUCT RES_TREE_HEADER
{
    RES_CHUNK_HEADER Header;
    RES_STRING_POOL_HEADER StringPool;
};

STRUCT RES_STYLE_POOL_ITEM
{
    CHAR* Name;
    UINT FirstChar, LastChar;
    RES_STYLE_POOL_ITEM* Next;
};

class DLLEXPORT cBinXMLFile: public cFile
{
public:
    cBinXMLFile(CHAR* Filename);
    cBinXMLFile(CHAR* Buffer, DWORD Size);
    ~cBinXMLFile();

    BOOL isReady;

private:
    BOOL ProcessXML();

    void ParseStringPool(RES_CHUNK_HEADER* Header);
    void ParseResourceMapping(RES_CHUNK_HEADER* Header);

    void ParseXMLStart(RES_CHUNK_HEADER* Header);
    void ParseXMLEnd(RES_CHUNK_HEADER* Header);

    void ParseStartNameSpace(RES_CHUNK_HEADER* Header);
    void ParseEndNameSpace(RES_CHUNK_HEADER* Header);

    UINT ResourceMapSize;
    UINT* ResourceMap;

    CHAR** StringPool;
    UINT StringPoolSize;

    RES_STYLE_POOL_ITEM** StylePool;
    UINT StylePoolSize;
};