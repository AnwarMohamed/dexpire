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

#include "cDexFile.h"
#define NO_INDEX 0xffffffff

cDexFile::cDexFile(CHAR* Filename): 
    cFile(Filename)
{
    isReady = BaseAddress && FileLength >= sizeof(DEX_HEADER) && DumpDex();
}

void cDexFile::DumpClassInfo(
    UINT ClassIndex, 
    DEX_CLASS_STRUCTURE* Class
    )
{
    (*Class).Descriptor = StringItems[DexTypeIds[DexClassDefs[ClassIndex].ClassIdx].StringIndex].Data;
    (*Class).AccessFlags = DexClassDefs[ClassIndex].AccessFlags;
    (*Class).SuperClass = StringItems[DexTypeIds[DexClassDefs[ClassIndex].SuperclassIdx].StringIndex].Data;

    if (DexClassDefs[ClassIndex].SourceFileIdx != NO_INDEX)
        (*Class).SourceFile = StringItems[DexClassDefs[ClassIndex].SourceFileIdx].Data;
    else
        (*Class).SourceFile = (UCHAR*)"No Information Found";
}

void cDexFile::DumpClassDataInfo(
    UINT ClassIndex, 
    DEX_CLASS_STRUCTURE* Class, 
    UCHAR** Buffer
    )
{
    /* Fields & Methods Sizes */
    (*Class).ClassData->StaticFieldsSize = ReadUnsignedLeb128((const UCHAR**)Buffer);
    (*Class).ClassData->InstanceFieldsSize = ReadUnsignedLeb128((const UCHAR**)Buffer);
    (*Class).ClassData->DirectMethodsSize = ReadUnsignedLeb128((const UCHAR**)Buffer);    
    (*Class).ClassData->VirtualMethodsSize = ReadUnsignedLeb128((const UCHAR**)Buffer);
    (*Class).ClassData->InterfacesSize = 
        DexClassDefs[ClassIndex].InterfacesOff? *(UINT*)(BaseAddress + DexClassDefs[ClassIndex].InterfacesOff): 0;
}

void cDexFile::AllocateClassData(
    UINT ClassIndex, 
    DEX_CLASS_STRUCTURE* Class
    )
{
    /* Allocating Static Fields */
    (*Class).ClassData->StaticFields = 
        new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD[(*Class).ClassData->StaticFieldsSize];
    memset((*Class).ClassData->StaticFields, NULL, 
        (*Class).ClassData->StaticFieldsSize * sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD));

    /* Allocating Instance Fields */
    (*Class).ClassData->InstanceFields = 
        new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD[(*Class).ClassData->InstanceFieldsSize];
    memset((*Class).ClassData->InstanceFields, NULL, 
        (*Class).ClassData->InstanceFieldsSize * sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD));

    /* Allocating Direct Methods */
    (*Class).ClassData->DirectMethods = 
        new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD[(*Class).ClassData->DirectMethodsSize];
    memset((*Class).ClassData->DirectMethods, NULL, 
        (*Class).ClassData->DirectMethodsSize * sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD));

    /* Allocating Virtual Methods */
    (*Class).ClassData->VirtualMethods = 
        new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD[(*Class).ClassData->VirtualMethodsSize];
    memset((*Class).ClassData->VirtualMethods, NULL, 
        (*Class).ClassData->VirtualMethodsSize * sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD));
            
    /* Allocating Interfaces */
    (*Class).ClassData->Interfaces = new UCHAR*[(*Class).ClassData->InterfacesSize];
    memset((*Class).ClassData->Interfaces, NULL, (*Class).ClassData->InterfacesSize * sizeof(UCHAR*));
}

void cDexFile::DumpFieldByIndex(
    UINT FieldIndex, 
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD* Field, 
    UCHAR** Buffer
    )
{
    (*Field).Type = StringItems[DexTypeIds[ DexFieldIds[FieldIndex].TypeIdex ].StringIndex].Data;
    (*Field).Name = StringItems[DexFieldIds[FieldIndex].StringIndex].Data;
    (*Field).AccessFlags = ReadUnsignedLeb128((const UCHAR**)Buffer);
} 

void cDexFile::DumpInterfaceByIndex(
    UINT ClassIndex, 
    UINT InterfaceIndex, 
    UCHAR** Interface
    )
{
    (*Interface) = StringItems[DexTypeIds[ 
        ((USHORT*)(BaseAddress+DexClassDefs[ClassIndex].InterfacesOff+sizeof(UINT)))[InterfaceIndex] 
    ].StringIndex].Data;
}

BOOL cDexFile::DumpDex()
{
    UCHAR* BufPtr, * BufPtr2;
    DexHeader = (DEX_HEADER*)BaseAddress;

    //check for magic code for opt header
    if (memcmp(DexHeader->Magic, DEX_MAGIC, 4) != 0)
        return FALSE;

    memcpy_s((UCHAR*)DexVersion, 4, (UCHAR*)DexHeader->Magic + 4, 4);

    if (FileLength != DexHeader->FileSize)
        return FALSE;

    /* Start String Items */
    nStringIDs = DexHeader->StringIdsSize;
    nStringItems = nStringIDs;
    DexStringIds = (DEX_STRING_ID*)(BaseAddress + DexHeader->StringIdsOff);
    StringItems = (DEX_STRING_ITEM*)malloc(nStringItems * sizeof(DEX_STRING_ITEM));
    memset(StringItems, NULL, nStringItems * sizeof(DEX_STRING_ITEM));

    for (UINT i=0; i<nStringIDs; i++)
    {
        BufPtr = (UCHAR*)BaseAddress + DexStringIds[i].StringDataOff;
        StringItems[i].StringSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
        StringItems[i].Data = BufPtr;
    }
    /* End String Items */

    /* Start Field IDs */
    nFieldIDs = DexHeader->FieldIdsSize;
    DexFieldIds = (DEX_FIELD_ID*)(BaseAddress + DexHeader->FieldIdsOff);
    /* End Field IDs */

    /* Start Type IDs */
    nTypeIDs = DexHeader->TypeIdsSize;
    DexTypeIds = (DEX_TYPE_ID*)(BaseAddress + DexHeader->TypeIdsOff);
    /* End Type IDs */

    /* Start Method IDs */
    nMethodIDs = DexHeader->MethodIdsSize;
    DexMethodIds = (DEX_METHOD_ID*)(BaseAddress + DexHeader->MethodIdsOff);
    /* End Method IDs */

    /* Start Prototype IDs */
    nPrototypeIDs = DexHeader->ProtoIdsSize;
    DexProtoIds = (DEX_PROTO_ID*)(BaseAddress + DexHeader->ProtoIdsOff);
    /* End Prototype IDs */

    /* Start Class Definitions */
    nClassDefinitions = DexHeader->ClassDefsSize;
    nClasses = nClassDefinitions;

    DexClasses = (DEX_CLASS_STRUCTURE*)malloc(nClasses * sizeof(DEX_CLASS_STRUCTURE));
    memset(DexClasses, NULL, nClasses * sizeof(DEX_CLASS_STRUCTURE));
    DexClassDefs = (DEX_CLASS_DEF*)(BaseAddress + DexHeader->ClassDefsOff);

    for (UINT i=0; i<nClasses; i++)
    {
        DumpClassInfo(i, &DexClasses[i]);

        if (DexClassDefs[i].ClassDataOff != NULL)
        {
            DexClasses[i].ClassData = new DEX_CLASS_STRUCTURE::CLASS_DATA;
            DexClassData = (DEX_CLASS_DATA*)(BaseAddress + DexClassDefs[i].ClassDataOff);

            BufPtr = (UCHAR*)DexClassData;

            DumpClassDataInfo(i, &DexClasses[i], &BufPtr);
            
            AllocateClassData(i, &DexClasses[i]); 


            /* Parsing Interfaces */
            UINT CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->InterfacesSize; j++)
                DumpInterfaceByIndex(
                i,
                j,
                &DexClasses[i].ClassData->Interfaces[j]);

            /* Parsing Static Fields */
            for (UINT j=0; j<DexClasses[i].ClassData->StaticFieldsSize; j++)
                DumpFieldByIndex(
                    (CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr)),
                    &DexClasses[i].ClassData->StaticFields[j], 
                    &BufPtr);

            /* Parsing Instance Fields */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->InstanceFieldsSize; j++)
                DumpFieldByIndex(
                    (CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr)),
                    &DexClasses[i].ClassData->InstanceFields[j], 
                    &BufPtr);

            /* Parsing Direct Methods */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->DirectMethodsSize; j++)
            {
                CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
                DexClasses[i].ClassData->DirectMethods[j].ProtoType = 
                    StringItems[DexTypeIds[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].ReturnTypeIdx].StringIndex].Data;

                /* Parsing Parameters */
                DumpMethodParameters(
                    CurIndex, 
                    &DexClasses[i].ClassData->DirectMethods[j]);
                

                DexClasses[i].ClassData->DirectMethods[j].Name = StringItems[DexMethodIds[CurIndex].StringIndex].Data;
                DexClasses[i].ClassData->DirectMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);

                UINT code_offset = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
                if (code_offset == NULL)
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea = NULL;
                else
                {
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE;
                    memset(DexClasses[i].ClassData->DirectMethods[j].CodeArea, NULL, 
                        sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE));

                    DexCode = (DEX_CODE*)(BaseAddress + code_offset);

                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->InsSize = DexCode->InsSize;
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->RegistersSize = DexCode->RegistersSize;
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->OutsSize = DexCode->OutsSize;
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize = DexCode->TriesSize;
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->InstructionsSize = DexCode->InstructionsSize;

                    /* Start Debug Info */
                    BufPtr2 = (UCHAR*)(BaseAddress + DexCode->DebugInfoOff);

                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.LineStart = 
                        ReadUnsignedLeb128((const UCHAR**)&BufPtr2);
                    DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersSize = 
                        ReadUnsignedLeb128((const UCHAR**)&BufPtr2);

                    if (DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersSize > 0)
                    {
                        DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersNames = 
                            new UCHAR*[DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersSize];
                        memset(DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersNames, NULL, 
                            DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersSize * sizeof(UCHAR*));

                        UINT test;
                        for (UINT k=0; k<DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersSize; k++)
                        {
                            //DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersNames[k] = 
                            //  StringItems[ ReadUnsignedLeb128((const UCHAR**)(ParametersNames + k*sizeof(UINT))) ].Data;
                            //test = ReadUnsignedLeb128((const UCHAR**)(BufPtr2));
                        }
                    }
                    else
                        DexClasses[i].ClassData->DirectMethods[j].CodeArea->DebugInfo.ParametersNames = NULL;

                    /* End Debug Info */


                    /* Tries Parsing */
                    DumpMethodTryItems(DexClasses[i].ClassData->DirectMethods[j].CodeArea, DexCode->TriesSize);

                    /* Start Instructions Parsing */
                    if (DexCode->InstructionsSize > 0)
                    {
                        DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions = 
                            new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION[DexCode->InstructionsSize];
                    }
                    else
                        DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions = NULL;

                    /* End Instructions Parsing */
                }

            }

            /* Parsing Virtual Methods */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->VirtualMethodsSize; j++)
            {
                CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
                DexClasses[i].ClassData->VirtualMethods[j].ProtoType = 
                    StringItems[DexTypeIds[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].ReturnTypeIdx].StringIndex].Data;

                //DexClasses[i].ClassData->VirtualMethods[j].Type = 
                //  StringItems[DexTypeIds[DexMethodIds[CurIndex].ClassIndex].StringIndex].Data;

                DexClasses[i].ClassData->VirtualMethods[j].Name = StringItems[DexMethodIds[CurIndex].StringIndex].Data;
                DexClasses[i].ClassData->VirtualMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);

                UINT code_offset = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
                if (code_offset == NULL)
                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea = NULL;
                else
                {
                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE;
                    DexCode = (DEX_CODE*)(BaseAddress + code_offset);

                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InsSize = DexCode->InsSize;
                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea->RegistersSize = DexCode->RegistersSize;
                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea->OutsSize = DexCode->OutsSize;
                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize = DexCode->TriesSize;
                    DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InstructionsSize = DexCode->InstructionsSize;

                    //DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries;
                    //DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions;
                }
            }
        }
    }

    /* End Class Definitions */

    return TRUE;
}

void cDexFile::DumpMethodParameters(
    UINT MethodIndex, 
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method
    )
{
    if (DexProtoIds[DexMethodIds[MethodIndex].PrototypeIndex].ParametersOff)
    {
        UINT ParamStringLen = 0;
        DEX_TYPE_LIST* ParametersList = (DEX_TYPE_LIST*)(BaseAddress + DexProtoIds[DexMethodIds[MethodIndex].PrototypeIndex].ParametersOff);
                
        for (UINT k=0; k<ParametersList->Size; k++) 
            ParamStringLen += strlen((CHAR*)StringItems[DexTypeIds[ParametersList->List[k].TypeIdx].StringIndex].Data);

        (*Method).Type = new UCHAR[ParamStringLen+1];

        ParamStringLen = 0;
        for (UINT k=0; k<ParametersList->Size; k++)
        {
            memcpy((*Method).Type + ParamStringLen, 
                (CHAR*)StringItems[DexTypeIds[ParametersList->List[k].TypeIdx].StringIndex].Data,
                StringItems[DexTypeIds[ParametersList->List[k].TypeIdx].StringIndex].StringSize);

            ParamStringLen += StringItems[DexTypeIds[ParametersList->List[k].TypeIdx].StringIndex].StringSize;
        }
        (*Method).Type[ParamStringLen] = '\0';
    }
    else
        (*Method).Type = (UCHAR*)"";
}

void cDexFile::DumpMethodTryItems(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE* CodeArea,
    UINT Size
    )
{
    if (Size > 0)
    {
        (*CodeArea).Tries = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_TRY[Size];
        memset((*CodeArea).Tries, NULL, 
            sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_TRY) * DexCode->TriesSize);
                        
        USHORT* InstructionsEnd = &((DexCode->Instructions)[DexCode->InstructionsSize]);
        if ((((UINT)InstructionsEnd) & 3) != 0) { InstructionsEnd++; }
        DEX_TRY_ITEM* TryItems = (DEX_TRY_ITEM*)InstructionsEnd;

        /* Parsing Catch Handlers */
        UCHAR* Buffer = (UCHAR*)&(TryItems[DexCode->TriesSize]);
        DumpMethodCatchHandlers(
            CodeArea, 
            &Buffer);

        for (UINT k=0; k<DexCode->TriesSize; k++)
            DumpMethodTryItemsInfo(
                &(*CodeArea).Tries[k],
                &TryItems[k],
                &(*CodeArea).CatchHandlers
            );
    }
    else 
        (*CodeArea).Tries = NULL;
}

void cDexFile::DumpMethodCatchHandlers(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE* CodeArea, 
    UCHAR** Buffer
    )
{
    (*CodeArea).CatchHandlersSize = ReadUnsignedLeb128((const UCHAR**)Buffer);
    (*CodeArea).CatchHandlers = 
        new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER
        [(*CodeArea).CatchHandlersSize];

    for (UINT k=0; k<(*CodeArea).CatchHandlersSize; k++)
    {
        (*CodeArea).CatchHandlers[k].TypeHandlersSize = ReadSignedLeb128((const UCHAR**)Buffer);
        (*CodeArea).CatchHandlers[k].TypeHandlers = 
            new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER::CLASS_CODE_CATCH_TYPE_PAIR
            [abs((*CodeArea).CatchHandlers[k].TypeHandlersSize)];

        for (UINT l=0; l<(UINT)abs((*CodeArea).CatchHandlers[k].TypeHandlersSize); l++)
        {
            (*CodeArea).CatchHandlers[k].TypeHandlers[l].TypeIndex = ReadUnsignedLeb128((const UCHAR**)Buffer);
            (*CodeArea).CatchHandlers[k].TypeHandlers[l].Address = ReadUnsignedLeb128((const UCHAR**)Buffer);

            if ((*CodeArea).CatchHandlers[k].TypeHandlers[l].TypeIndex == NO_INDEX)
                (*CodeArea).CatchHandlers[k].TypeHandlers[l].Type = (UCHAR*)"<any>";
            else
                (*CodeArea).CatchHandlers[k].TypeHandlers[l].Type =
                    StringItems[DexTypeIds[ (*CodeArea).CatchHandlers[k].TypeHandlers[l].TypeIndex ].StringIndex].Data;
        }

        if ((*CodeArea).CatchHandlers[k].TypeHandlersSize > 0)
            (*CodeArea).CatchHandlers[k].CatchAllAddress = NULL;
        else
            (*CodeArea).CatchHandlers[k].CatchAllAddress = ReadUnsignedLeb128((const UCHAR**)Buffer);

        (*CodeArea).CatchHandlers[k].TypeHandlersSize = abs((*CodeArea).CatchHandlers[k].TypeHandlersSize);
    }
}

void cDexFile::DumpMethodTryItemsInfo(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_TRY* TryItem, 
    DEX_TRY_ITEM* TryItemInfo,
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER** CatchHandlers
    )
{
    (*TryItem).InstructionsStart = (*TryItemInfo).StartAddress;
    (*TryItem).InstructionsEnd = (*TryItemInfo).StartAddress + (*TryItemInfo).InstructionsSize;
    (*TryItem).CatchHandler = &(*CatchHandlers)[(*TryItemInfo).HandlerOff-1];
}

CHAR* cDexFile::GetAccessMask(
    UINT Type, 
    UINT AccessFlags
    )
{
    CHAR* str,* cp;
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

INT cDexFile::ReadSignedLeb128(
    const UCHAR** pStream
    ) 
{
 const UCHAR* ptr = *pStream;
    int result = *(ptr++);

    if (result <= 0x7f) 
    {
        result = (result << 25) >> 25;
    } 
    else 
    {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur <= 0x7f) {
            result = (result << 18) >> 18;
        } 
        else 
        {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur <= 0x7f) 
            {
                result = (result << 11) >> 11;
            } 
            else 
            {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur <= 0x7f) 
                {
                    result = (result << 4) >> 4;
                } 
                else 
                {                
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }

    *pStream = ptr;
    return result;
}

INT cDexFile::ReadUnsignedLeb128(
    const UCHAR** pStream
    ) 
{
    const UCHAR* ptr = *pStream;
    int result = *(ptr++);

    if (result > 0x7f) 
    {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) 
        {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) 
            {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) 
                {
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }

    *pStream = ptr;
    return result;
};

cDexFile::~cDexFile(void)
{
}
