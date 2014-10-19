/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#include "cBinXMLFile.h"
#include <stdio.h>

cBinXMLFile::cBinXMLFile(CHAR* Filename):
    cFile(Filename)
{
    isReady = BaseAddress && ProcessXML();
}

cBinXMLFile::cBinXMLFile(CHAR* Buffer, DWORD Size):
    cFile(Buffer, Size)
{
    isReady = BaseAddress && ProcessXML();
}

BOOL cBinXMLFile::ProcessXML()
{
    RES_CHUNK_HEADER* ChunkHeader = (RES_CHUNK_HEADER*)BaseAddress;

    ResourceMap = NULL;
    ResourceMapSize = 0;

    StringPool = NULL;
    StringPoolSize = 0;

    if (ChunkHeader->Type != RES_XML_TYPE)
        return FALSE;

    for (UINT i=ChunkHeader->HeaderSize; i<FileLength;)
    {
        ChunkHeader = (RES_CHUNK_HEADER*)(BaseAddress + i);
        
        if (ChunkHeader->Type == RES_STRING_POOL_TYPE)
            ParseStringPool(ChunkHeader);

        else if (ChunkHeader->Type == RES_XML_RESOURCE_MAP_TYPE)
            ParseResourceMapping(ChunkHeader);

        else if (ChunkHeader->Type == RES_XML_START_NAMESPACE_TYPE)
            ParseStartNameSpace(ChunkHeader);

        else if (ChunkHeader->Type == RES_XML_START_ELEMENT_TYPE)
            ParseXMLStart(ChunkHeader);

        else if (ChunkHeader->Type == RES_XML_END_ELEMENT_TYPE)
            ParseXMLEnd(ChunkHeader);

        else if (ChunkHeader->Type == RES_XML_END_NAMESPACE_TYPE)
            ParseEndNameSpace(ChunkHeader);
                
        i+= ChunkHeader->Size;
    }

    return TRUE;
}

void cBinXMLFile::ParseEndNameSpace(
    RES_CHUNK_HEADER* Header
    )
{
}

void cBinXMLFile::ParseStartNameSpace(
    RES_CHUNK_HEADER* Header
    )
{
}

void cBinXMLFile::ParseXMLStart(
    RES_CHUNK_HEADER* Header
    )
{
}

void cBinXMLFile::ParseXMLEnd(
    RES_CHUNK_HEADER* Header
    )
{
}

void cBinXMLFile::ParseResourceMapping(
    RES_CHUNK_HEADER* Header
    )
{
    ResourceMapSize = (Header->Size-8)/4;

    if (ResourceMapSize)
        ResourceMap = new UINT[ResourceMapSize];

    for (UINT i=0; i<ResourceMapSize; i++)
        ResourceMap[i] = *(UINT*)((DWORD)Header + Header->HeaderSize + i*4);
}

void cBinXMLFile::ParseStringPool(
    RES_CHUNK_HEADER* Header
    )
{
    RES_STRING_POOL_HEADER* StringPoolHeader;
    UINT *StringIndices=NULL, *StyleIndices=NULL;

    StringPoolHeader = (RES_STRING_POOL_HEADER*)((DWORD)Header + Header->HeaderSize);

    if (StringPoolHeader->StringCount)
    {
        StringPoolSize = StringPoolHeader->StringCount;
        StringPool = new CHAR*[StringPoolHeader->StringCount];
        StringIndices = new UINT[StringPoolHeader->StringCount];
        memcpy(
            StringPoolHeader + sizeof(RES_STRING_POOL_HEADER), 
            StringIndices, StringPoolHeader->StringCount*4);
    }

    if (StringPoolHeader->StyleCount)
    {
        StylePoolSize = StringPoolHeader->StyleCount;
        StylePool = new RES_STYLE_POOL_ITEM*[StringPoolHeader->StyleCount];
        StyleIndices = new UINT[StringPoolHeader->StyleCount];
        memcpy(
            StringPoolHeader + sizeof(RES_STRING_POOL_HEADER) + StringPoolHeader->StringCount*4, 
            StyleIndices, StringPoolHeader->StyleCount*4);
    }

    UINT StringLength, ActualStringLength;
    CHAR* StringData = (CHAR*)((DWORD)Header + StringPoolHeader->StringStart);

    for (UINT i=0; i<StringPoolHeader->StringCount; i++)
    {
        if (i+1 == StringPoolHeader->StringCount)
        {
            if (!StringPoolHeader->StylesStart)
                StringLength = Header->Size - StringIndices[i] - Header->HeaderSize - 4*StringPoolHeader->StringCount;
            else 
                StringLength = StringPoolHeader->StylesStart - StringIndices[i];
        }
        else
            StringLength = StringIndices[i+1] - StringIndices[i];
    
        if (StringData[0] == StringData[1])
            ActualStringLength = StringData[0];
        else
            ActualStringLength = *(USHORT*)StringData;

        StringData+= 2;

        CHAR* String = new CHAR[ActualStringLength];
        UINT StringIndex = 0;

        for (UINT j=0; j<ActualStringLength; j++)
        {
            if (*StringData++ != 0x00)
                String[StringIndex++] = *(StringData-1);
        }

        StringPool[i] = new CHAR[strlen(String)+1];
        memcpy(StringPool[i], String, strlen(String));
        StringPool[i][strlen(String)] = NULL;

        delete[] String;
        printf("String: %s", String);
    }

    UINT* StyleData = (UINT*)((DWORD)Header+ StringPoolHeader->StylesStart);

    for (UINT i=0; i<StringPoolHeader->StyleCount; i++)
    {
        if (*(UINT*)(StyleData++) == 0xFFFFFFFF)
            StylePool[i] = NULL;
        else
        {
            /*
            StylePool[i] = new RES_STYLE_POOL_ITEM;
            printf("%ud", *(--StyleData)++);
            StylePool[i]->Name = StringPool[*StyleData++];
            StylePool[i]->FirstChar = *StyleData++;
            StylePool[i]->LastChar = *StyleData++;

            while(*(UINT*)(StyleData+=4) != 0xFFFFFFFF)
            {

            }
            */
        }
    }

    if (StyleIndices)
        delete[] StyleIndices;

    if (StringIndices)
        delete[] StringIndices;
}

cBinXMLFile::~cBinXMLFile()
{
}