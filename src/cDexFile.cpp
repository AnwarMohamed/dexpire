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
#include <stdio.h>
#define NO_INDEX 0xffffffff

cDexFile::cDexFile(CHAR* Filename): 
    cFile(Filename)
{
    CreateOpcodesWidthsTable();
    CreateOpcodesFlagsTable();
    CreateOpcodesFormatTable();

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
        (*Class).SourceFile = 0;
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
    UCHAR* BufPtr;
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
                DumpMethodById(
                    (CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr)),
                    &DexClasses[i].ClassData->DirectMethods[j],
                    &BufPtr);

            /* Parsing Virtual Methods */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->VirtualMethodsSize; j++)
                DumpMethodById(
                    (CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr)),
                    &DexClasses[i].ClassData->VirtualMethods[j],
                    &BufPtr);

            /* Parsing Annotations */
            DumpAnnotations(&DexClasses[i], DexClassDefs[i].AnnotationsOff);
        }
    }

    return TRUE;
}

void cDexFile::DumpAnnotations(
    DEX_CLASS_STRUCTURE* DexClass, 
    UINT Offset
    )
{
    if (!Offset) return;
    DEX_ANNOTATIONS_DIRECTORY_ITEM* Annotations = (DEX_ANNOTATIONS_DIRECTORY_ITEM*)(BaseAddress + Offset);
    UCHAR* Ptr;
    DEX_ANNOTATION_ITEM* ClassAnnotationItem;
    DEX_ANNOTATION_SET_ITEM* ClassAnnotations;

    if (Annotations->ClassAnnotationsOff)
    {
        ClassAnnotations = (DEX_ANNOTATION_SET_ITEM*)(BaseAddress + Annotations->ClassAnnotationsOff);
        DexClass->ClassData->AnnotationsSize = ClassAnnotations->Size;
        DexClass->ClassData->Annotations = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_ANNOTATION[DexClass->ClassData->AnnotationsSize];
        memset(DexClass->ClassData->Annotations, NULL, DexClass->ClassData->AnnotationsSize * sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_ANNOTATION));

        for (UINT i=0; i<ClassAnnotations->Size; i++)
        {
            ClassAnnotationItem = (DEX_ANNOTATION_ITEM*)(BaseAddress + ClassAnnotations->Entries[i].AnnotationOff);
            Ptr = (UCHAR*)ClassAnnotationItem->Encoded;

            DexClass->ClassData->Annotations[i].Type = StringItems[DexTypeIds[ReadUnsignedLeb128((const UCHAR**)&Ptr)].StringIndex].Data;
            DexClass->ClassData->Annotations[i].Visibility = ClassAnnotationItem->Visibility;
            DexClass->ClassData->Annotations[i].ElementsSize = ReadUnsignedLeb128((const UCHAR**)&Ptr);

            if (DexClass->ClassData->Annotations[i].ElementsSize)
            {
                DexClass->ClassData->Annotations[i].Elements = 
                    new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_ANNOTATION::CLASS_ANNOTATION_ELEMENT[DexClass->ClassData->Annotations[i].ElementsSize];
                memset(DexClass->ClassData->Annotations[i].Elements, NULL, 
                    DexClass->ClassData->Annotations[i].ElementsSize * sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_ANNOTATION::CLASS_ANNOTATION_ELEMENT));
            }

            for (UINT j=0; j<DexClass->ClassData->Annotations[i].ElementsSize; j++)
            {
                DexClass->ClassData->Annotations[i].Elements[j].Name = StringItems[ReadUnsignedLeb128((const UCHAR**)&Ptr)].Data;
                DexClass->ClassData->Annotations[i].Elements[j].ValueType = *Ptr++;
                DexClass->ClassData->Annotations[i].Elements[j].ValueSize = (DexClass->ClassData->Annotations[i].Elements[j].ValueType << 5) +1;
                DexClass->ClassData->Annotations[i].Elements[j].Value = Ptr;
                Ptr+= DexClass->ClassData->Annotations[i].Elements[j].ValueSize;
                int n=0;
            }
        }
    }
}

void cDexFile::DumpMethodById(
    UINT MethodIndex,
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method,
    UCHAR** Buffer)
{
    (*Method).ProtoType = StringItems[DexTypeIds[DexProtoIds[DexMethodIds[
        MethodIndex
    ].PrototypeIndex].ReturnTypeIdx].StringIndex].Data;

    /* Parsing Parameters */
    DumpMethodParameters(
        MethodIndex, 
        Method);
                
    (*Method).Name = StringItems[DexMethodIds[MethodIndex].StringIndex].Data;
    (*Method).AccessFlags = ReadUnsignedLeb128((const UCHAR**)Buffer);

    /* Parse Method Code */
    DumpMethodCode(
        (DEX_CODE*)(BaseAddress + ReadUnsignedLeb128((const UCHAR**)Buffer)),
        Method);
}

void cDexFile::DumpMethodCode(
    DEX_CODE* CodeAreaDef,
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method
    )
{
    if ((DWORD)CodeAreaDef == BaseAddress)
        (*Method).CodeArea = NULL;
    else
    {
        (*Method).CodeArea = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE;
        memset((*Method).CodeArea, NULL, sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE));

        DumpMethodCodeInfo(
            (*Method).CodeArea,
            CodeAreaDef);
 
        /* Debug Info */
        TempBuffer = (UCHAR*)(BaseAddress + (*CodeAreaDef).DebugInfoOff);
        DumpMethodDebugInfo(
            &((*Method).CodeArea->DebugInfo),
            &TempBuffer);
                  
        /* Tries Parsing */
        DumpMethodTryItems(
            (*Method).CodeArea, 
            CodeAreaDef);

        /* Instructions Parsing */
        DumpMethodInstructions(
            (*Method).CodeArea,
            CodeAreaDef);
    }
}

void cDexFile::DumpMethodCodeInfo(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef    
    )
{
    (*CodeArea).InsSize = (*CodeAreaDef).InsSize;
    (*CodeArea).RegistersSize = (*CodeAreaDef).RegistersSize;
    (*CodeArea).OutsSize = (*CodeAreaDef).OutsSize;
    (*CodeArea).TriesSize = (*CodeAreaDef).TriesSize;
    (*CodeArea).InstBufferSize = (*CodeAreaDef).InstructionsSize;
}

void cDexFile::DumpMethodDebugInfo(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_DEBUG_INFO* DebugInfo,
    UCHAR** Buffer
    )
{
    (*DebugInfo).LineStart = ReadUnsignedLeb128((const UCHAR**)Buffer);
    (*DebugInfo).ParametersSize = ReadUnsignedLeb128((const UCHAR**)Buffer);

    if ((*DebugInfo).ParametersSize > 0)
    {
        (*DebugInfo).ParametersNames = new UCHAR*[(*DebugInfo).ParametersSize];
        memset((*DebugInfo).ParametersNames, NULL, (*DebugInfo).ParametersSize * sizeof(UCHAR*));

        UINT ParameterIndex;
        for (UINT k=0; k<(*DebugInfo).ParametersSize; k++)
        {
            ParameterIndex = ReadUnsignedLeb128((const UCHAR**)Buffer) - 1;

            //if(ParameterIndex == NO_INDEX)
            //    (*DebugInfo).ParametersNames[k] = (UCHAR*)"";
            //else
            //    (*DebugInfo).ParametersNames[k] = StringItems[ParameterIndex].Data;
        }


    }
    else
        (*DebugInfo).ParametersNames = NULL;
}

void cDexFile::DumpMethodInstructions(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef
    )
{
    if ((*CodeAreaDef).InstructionsSize > 0)
    {
        
        (*CodeArea).Instructions = 
            (DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION**)
            malloc( (*CodeArea).InstructionsSize * 
            sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION*));
        
        (*CodeArea).Offset = (DWORD)(CodeAreaDef) - BaseAddress;

        CHAR Opcode;
        for (UINT i=0; i<(*CodeAreaDef).InstructionsSize*2;)
        {
            Opcode = ((CHAR*)((*CodeAreaDef).Instructions))[i];

            //UINT Width = OpcodesWidths[Opcode];
            //UINT Format = OpcodesFormat[Opcode];
            //UINT Flags = OpcodesFlags[Opcode];

            (*CodeArea).Instructions = 
                (DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION**)
                realloc((*CodeArea).Instructions, ++(*CodeArea).InstructionsSize * 
                sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION*));

            (*CodeArea).Instructions[(*CodeArea).InstructionsSize-1] = DecodeOpcode(((UCHAR*)((*CodeAreaDef).Instructions)) + i);
            (*CodeArea).Instructions[(*CodeArea).InstructionsSize-1]->Offset = 
                (DWORD)(CodeAreaDef->Instructions) - BaseAddress + i/2;

            i+= (*CodeArea).Instructions[(*CodeArea).InstructionsSize-1]->BytesSize; 
        }
    }
    else
        (*CodeArea).Instructions = NULL;
}

#define TEMP_STRING_SIZE 200

DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION*
    cDexFile::DecodeOpcode(
    UCHAR* Opcode
    )
{
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION* Decoded =
        new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION;
    memset(Decoded, 0, sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_INSTRUCTION));

    Decoded->Opcode = (UCHAR*)OpcodesStrings[Opcode[0]];
    Decoded->Format = (UCHAR*)OpcodesFormatStrings[OpcodesFormat[Opcode[0]]];
    Decoded->Bytes = (UCHAR*)Opcode;
    Decoded->BytesSize = 1;

    CHAR* TempString = new CHAR[TEMP_STRING_SIZE];

    switch(OpcodesFormat[Opcode[0]])
    {
    case OP_FORMAT_UNKNOWN:
        sprintf_s(TempString, TEMP_STRING_SIZE, "<unknown>");
        Decoded->BytesSize += 1;
        break;

    case OP_FORMAT_10x:        // op
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", 
            Decoded->Opcode);
        Decoded->BytesSize += 1;
        break;

    case OP_FORMAT_12x:        // op vA, vB
        Decoded->vA = Opcode[1] & 0xF0;
        Decoded->vB = Opcode[1] >> 4;
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_11n:        // op vA, #+B
        Decoded->vA = Opcode[1] & 0xF0;
        Decoded->vB = (CHAR)(Opcode[1] >> 4);
        Decoded->BytesSize += 1;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode, 
            Decoded->vA,
            (CHAR)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_11x:        // op vAA
        Decoded->vA = Opcode[1];
        Decoded->BytesSize += 1;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d", 
            Decoded->Opcode, 
            Decoded->vA);
        break;

    case OP_FORMAT_10t:        // op +AA
        Decoded->vA = (CHAR)Opcode[1];
        Decoded->BytesSize += 1;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s %s%d", 
            Decoded->Opcode, 
            (CHAR)Decoded->vA>=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_20bc:       // op AA, thing@BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s %d, %s@%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            "thing", 
            Decoded->vB);
        break;

    case OP_FORMAT_20t:        // op +AAAA
        Decoded->vA = *(SHORT*)&Opcode[1];
        Decoded->BytesSize += 1;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s %s%d", 
            Decoded->Opcode, 
            (SHORT)Decoded->vA>=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_22x:        // op vAA, vBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB);
        break;

    case OP_FORMAT_21t:        // op vAA, +BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, %s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21s:        // op vAA, #+BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21h:        // op vAA, #+BBBB00000[00000000]
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_21c:        // op vAA, thing@BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, %s@%d", 
            Decoded->Opcode, 
            Decoded->vA,
            Opcode[0] == OP_CONST_STRING? "string" :
            Opcode[0] == OP_CHECK_CAST || Opcode[0] == OP_CONST_CLASS? "class" :
            Opcode[0] == OP_NEW_INSTANCE? "type" :
            "field",
            Decoded->vB);
        break;

    case OP_FORMAT_23x:        // op vAA, vBB, vCC
        Decoded->vA = Opcode[1];
        Decoded->vB = Opcode[2];
        Decoded->vC = Opcode[3];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d, v%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            Decoded->vC);
        break;

    case OP_FORMAT_22b:        // op vAA, vBB, #+CC
        Decoded->vA = Opcode[1];
        Decoded->vB = Opcode[2];
        Decoded->vC = (CHAR)Opcode[3];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            (CHAR)Decoded->vC>=0?"+":"",
            Decoded->vC);
        break;

    case OP_FORMAT_22t:        // op vA, vB, +CCCC
        Decoded->vA = Opcode[1] & 0x0F;
        Decoded->vB = Opcode[1] >> 4;
        Decoded->vC = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d, %s%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            (SHORT)Decoded->vC>=0?"+":"",
            Decoded->vC);
        break;

    case OP_FORMAT_22s:        // op vA, vB, #+CCCC
        Decoded->vA = Opcode[1] & 0x0F;
        Decoded->vB = Opcode[1] >> 4;
        Decoded->vC = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            (SHORT)Decoded->vC>=0?"+":"",
            Decoded->vC);
        break;

    case OP_FORMAT_22c:        // op vA, vB, thing@CCCC
        Decoded->vA = Opcode[1] & 0x0F;
        Decoded->vB = Opcode[1] >> 4;
        Decoded->vC = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d, %s@%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            Opcode[0] >= OP_IGET && Opcode[0] <= OP_IPUT_SHORT? "field" :
            "class",
            Decoded->vC);
        break;

    case OP_FORMAT_22cs:       // [opt] op vA, vB, field offset CCCC
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_32x:        // op vAAAA, vBBBB
        Decoded->vA = *(USHORT*)&Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[3];
        Decoded->BytesSize += 5;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, v%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_30t:        // op +AAAAAAAA
        Decoded->vA = *(INT*)&Opcode[1];
        Decoded->BytesSize += 5;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s %s%d", 
            Decoded->Opcode,
            (INT)Decoded->vA<=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_31t:        // op vAA, +BBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(INT*)&Opcode[2];
        Decoded->BytesSize += 5;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, %s%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT)Decoded->vB<=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_31i:        // op vAA, #+BBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(INT*)&Opcode[2];
        Decoded->BytesSize += 5;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT)Decoded->vB<=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_31c:        // op vAA, thing@BBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(UINT*)&Opcode[2];
        Decoded->BytesSize += 5;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, string@%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_35c:        // op {vC, vD, vE, vF, vG}, thing@BBBB (B: count, A: vG)
        Decoded->vA = Opcode[1] >> 4;
        Decoded->vB = *(USHORT*)(&Opcode[2]);
        Decoded->vC = Decoded->vA>0? Opcode[4] & 0x0F :0;
        Decoded->vD = Decoded->vA>1? Opcode[4] >> 4 :0;
        Decoded->vE = Decoded->vA>2? Opcode[5] & 0x0F :0;
        Decoded->vF = Decoded->vA>3? Opcode[5] >> 4 :0;
        Decoded->vG = Decoded->vA==5? Opcode[6] & 0x0F :0;
        
        Decoded->BytesSize += 5;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s {", Decoded->Opcode);
        
        if (Decoded->vA)
            sprintf_s(TempString+strlen(TempString) , TEMP_STRING_SIZE-strlen(TempString), "v%d", Decoded->vC);
        if (Decoded->vA>1)
            sprintf_s(TempString+strlen(TempString) , TEMP_STRING_SIZE-strlen(TempString), " ,v%d", Decoded->vD);
        if (Decoded->vA>2)
            sprintf_s(TempString+strlen(TempString) , TEMP_STRING_SIZE-strlen(TempString), " ,v%d", Decoded->vE);
        if (Decoded->vA>3)
            sprintf_s(TempString+strlen(TempString) , TEMP_STRING_SIZE-strlen(TempString), " ,v%d", Decoded->vF);
        if (Decoded->vA==5)
            sprintf_s(TempString+strlen(TempString) , TEMP_STRING_SIZE-strlen(TempString), " ,v%d", Decoded->vG);

        sprintf_s(TempString+strlen(TempString) , TEMP_STRING_SIZE-strlen(TempString), "}, %s@%d",
            Opcode[0] == OP_FILLED_NEW_ARRAY? "class": "method",
            Decoded->vB);
        break;

    case OP_FORMAT_35ms:       // [opt] invoke-virtual+super
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_35fs:       // [opt] invoke-interface
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3rc:        // op {vCCCC .. v(CCCC+AA-1)}, meth@BBBB
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3rms:       // [opt] invoke-virtual+super/range
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3rfs:       // [opt] invoke-interface/range
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3inline:    // [opt] inline invoke
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_51l:        // op vAA, #+BBBBBBBBBBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB_wide = *(INT64*)&Opcode[2];
        Decoded->BytesSize += 9;
        sprintf_s(TempString, TEMP_STRING_SIZE, "%s v%d, #%s%I64d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT64)Decoded->vB_wide>=0?"+":"",
            Decoded->vB_wide);
        break;
    }

    Decoded->Decoded = new UCHAR[strlen(TempString)+1];
    strncpy_s((CHAR*)Decoded->Decoded, strlen(TempString)+1, TempString, TEMP_STRING_SIZE);
    free(TempString);
    return Decoded;
}



void cDexFile::DumpMethodParameters(
    UINT MethodIndex, 
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD* Method
    )
{
    if (DexProtoIds[DexMethodIds[MethodIndex].PrototypeIndex].ParametersOff)
    {
        UINT ParamStringLen = 0;
        DEX_TYPE_LIST* ParametersList = (DEX_TYPE_LIST*)(BaseAddress + DexProtoIds[DexMethodIds[
            MethodIndex
        ].PrototypeIndex].ParametersOff);
                
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
        (*Method).Type = 0;
}

void cDexFile::DumpMethodTryItems(
    DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef
    )
{
    if ((*CodeAreaDef).TriesSize > 0)
    {
        (*CodeArea).Tries = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_TRY[(*CodeAreaDef).TriesSize];
        memset((*CodeArea).Tries, NULL, 
            sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_TRY) * (*CodeAreaDef).TriesSize);
                        
        USHORT* InstructionsEnd = &(((*CodeAreaDef).Instructions)[(*CodeAreaDef).InstructionsSize]);
        if ((((UINT)InstructionsEnd) & 3) != 0) { InstructionsEnd++; }
        DEX_TRY_ITEM* TryItems = (DEX_TRY_ITEM*)InstructionsEnd;

        /* Parsing Catch Handlers */
        TempBuffer = (UCHAR*)&(TryItems[(*CodeAreaDef).TriesSize]);
        DumpMethodCatchHandlers(
            CodeArea, 
            &TempBuffer);

        for (UINT k=0; k<(*CodeAreaDef).TriesSize; k++)
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

    if ((*CodeArea).CatchHandlersSize)
        {
        (*CodeArea).CatchHandlers = 
            new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER
            [(*CodeArea).CatchHandlersSize];

        memset( (*CodeArea).CatchHandlers, NULL, 
            sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER) * (*CodeArea).CatchHandlersSize);

        for (UINT k=0; k<(*CodeArea).CatchHandlersSize; k++)
        {
            (*CodeArea).CatchHandlers[k].TypeHandlersSize = ReadSignedLeb128((const UCHAR**)Buffer);

            if ((*CodeArea).CatchHandlers[k].TypeHandlersSize)
            {
                (*CodeArea).CatchHandlers[k].TypeHandlers = 
                    new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER::CLASS_CODE_CATCH_TYPE_PAIR
                    [abs((*CodeArea).CatchHandlers[k].TypeHandlersSize)];

                memset((*CodeArea).CatchHandlers[k].TypeHandlers, NULL, 
                    sizeof(DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE::CLASS_CODE_CATCH_HANDLER::CLASS_CODE_CATCH_TYPE_PAIR) * 
                    abs((*CodeArea).CatchHandlers[k].TypeHandlersSize));

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
            }

            if ((*CodeArea).CatchHandlers[k].TypeHandlersSize > 0)
                (*CodeArea).CatchHandlers[k].CatchAllAddress = NULL;
            else
                (*CodeArea).CatchHandlers[k].CatchAllAddress = ReadUnsignedLeb128((const UCHAR**)Buffer);

            (*CodeArea).CatchHandlers[k].TypeHandlersSize = abs((*CodeArea).CatchHandlers[k].TypeHandlersSize);
        }
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

void cDexFile::CreateOpcodesFormatTable()
{
    OpcodesFormat = new CHAR[256];
    UCHAR Format;

    for (UINT i=0; i<256; i++) 
    {
        Format = OP_FORMAT_UNKNOWN;

        switch ((CHAR)i) 
        {
        case OP_GOTO:
            Format = OP_FORMAT_10t;
            break;
        case OP_NOP:
        case OP_RETURN_VOID:
            Format = OP_FORMAT_10x;
            break;
        case OP_CONST_4:
            Format = OP_FORMAT_11n;
            break;
        case OP_CONST_HIGH16:
        case OP_CONST_WIDE_HIGH16:
            Format = OP_FORMAT_21h;
            break;
        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
        case OP_MOVE_RESULT_OBJECT:
        case OP_MOVE_EXCEPTION:
        case OP_RETURN:
        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:
        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:
        case OP_THROW:
            Format = OP_FORMAT_11x;
            break;
        case OP_MOVE:
        case OP_MOVE_WIDE:
        case OP_MOVE_OBJECT:
        case OP_ARRAY_LENGTH:
        case OP_NEG_INT:
        case OP_NOT_INT:
        case OP_NEG_LONG:
        case OP_NOT_LONG:
        case OP_NEG_FLOAT:
        case OP_NEG_DOUBLE:
        case OP_INT_TO_LONG:
        case OP_INT_TO_FLOAT:
        case OP_INT_TO_DOUBLE:
        case OP_LONG_TO_INT:
        case OP_LONG_TO_FLOAT:
        case OP_LONG_TO_DOUBLE:
        case OP_FLOAT_TO_INT:
        case OP_FLOAT_TO_LONG:
        case OP_FLOAT_TO_DOUBLE:
        case OP_DOUBLE_TO_INT:
        case OP_DOUBLE_TO_LONG:
        case OP_DOUBLE_TO_FLOAT:
        case OP_INT_TO_BYTE:
        case OP_INT_TO_CHAR:
        case OP_INT_TO_SHORT:
        case OP_ADD_INT_2ADDR:
        case OP_SUB_INT_2ADDR:
        case OP_MUL_INT_2ADDR:
        case OP_DIV_INT_2ADDR:
        case OP_REM_INT_2ADDR:
        case OP_AND_INT_2ADDR:
        case OP_OR_INT_2ADDR:
        case OP_XOR_INT_2ADDR:
        case OP_SHL_INT_2ADDR:
        case OP_SHR_INT_2ADDR:
        case OP_USHR_INT_2ADDR:
        case OP_ADD_LONG_2ADDR:
        case OP_SUB_LONG_2ADDR:
        case OP_MUL_LONG_2ADDR:
        case OP_DIV_LONG_2ADDR:
        case OP_REM_LONG_2ADDR:
        case OP_AND_LONG_2ADDR:
        case OP_OR_LONG_2ADDR:
        case OP_XOR_LONG_2ADDR:
        case OP_SHL_LONG_2ADDR:
        case OP_SHR_LONG_2ADDR:
        case OP_USHR_LONG_2ADDR:
        case OP_ADD_FLOAT_2ADDR:
        case OP_SUB_FLOAT_2ADDR:
        case OP_MUL_FLOAT_2ADDR:
        case OP_DIV_FLOAT_2ADDR:
        case OP_REM_FLOAT_2ADDR:
        case OP_ADD_DOUBLE_2ADDR:
        case OP_SUB_DOUBLE_2ADDR:
        case OP_MUL_DOUBLE_2ADDR:
        case OP_DIV_DOUBLE_2ADDR:
        case OP_REM_DOUBLE_2ADDR:
            Format = OP_FORMAT_12x;
            break;
        case OP_GOTO_16:
            Format = OP_FORMAT_20t;
            break;
        case OP_GOTO_32:
            Format = OP_FORMAT_30t;
            break;
        case OP_CONST_STRING:
        case OP_CONST_CLASS:
        case OP_CHECK_CAST:
        case OP_NEW_INSTANCE:
        case OP_SGET:
        case OP_SGET_WIDE:
        case OP_SGET_OBJECT:
        case OP_SGET_BOOLEAN:
        case OP_SGET_BYTE:
        case OP_SGET_CHAR:
        case OP_SGET_SHORT:
        case OP_SPUT:
        case OP_SPUT_WIDE:
        case OP_SPUT_OBJECT:
        case OP_SPUT_BOOLEAN:
        case OP_SPUT_BYTE:
        case OP_SPUT_CHAR:
        case OP_SPUT_SHORT:
            Format = OP_FORMAT_21c;
            break;
        case OP_CONST_16:
        case OP_CONST_WIDE_16:
            Format = OP_FORMAT_21s;
            break;
        case OP_IF_EQZ:
        case OP_IF_NEZ:
        case OP_IF_LTZ:
        case OP_IF_GEZ:
        case OP_IF_GTZ:
        case OP_IF_LEZ:
            Format = OP_FORMAT_21t;
            break;
        case OP_FILL_ARRAY_DATA:
        case OP_PACKED_SWITCH:
        case OP_SPARSE_SWITCH:
            Format = OP_FORMAT_31t;
            break;
        case OP_ADD_INT_LIT8:
        case OP_RSUB_INT_LIT8:
        case OP_MUL_INT_LIT8:
        case OP_DIV_INT_LIT8:
        case OP_REM_INT_LIT8:
        case OP_AND_INT_LIT8:
        case OP_OR_INT_LIT8:
        case OP_XOR_INT_LIT8:
        case OP_SHL_INT_LIT8:
        case OP_SHR_INT_LIT8:
        case OP_USHR_INT_LIT8:
            Format = OP_FORMAT_22b;
            break;
        case OP_INSTANCE_OF:
        case OP_NEW_ARRAY:
        case OP_IGET:
        case OP_IGET_WIDE:
        case OP_IGET_OBJECT:
        case OP_IGET_BOOLEAN:
        case OP_IGET_BYTE:
        case OP_IGET_CHAR:
        case OP_IGET_SHORT:
        case OP_IPUT:
        case OP_IPUT_WIDE:
        case OP_IPUT_OBJECT:
        case OP_IPUT_BOOLEAN:
        case OP_IPUT_BYTE:
        case OP_IPUT_CHAR:
        case OP_IPUT_SHORT:
            Format = OP_FORMAT_22c;
            break;
        case OP_ADD_INT_LIT16:
        case OP_RSUB_INT:
        case OP_MUL_INT_LIT16:
        case OP_DIV_INT_LIT16:
        case OP_REM_INT_LIT16:
        case OP_AND_INT_LIT16:
        case OP_OR_INT_LIT16:
        case OP_XOR_INT_LIT16:
            Format = OP_FORMAT_22s;
            break;
        case OP_IF_EQ:
        case OP_IF_NE:
        case OP_IF_LT:
        case OP_IF_GE:
        case OP_IF_GT:
        case OP_IF_LE:
            Format = OP_FORMAT_22t;
            break;
        case OP_MOVE_FROM16:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_OBJECT_FROM16:
            Format = OP_FORMAT_22x;
            break;
        case OP_CMPL_FLOAT:
        case OP_CMPG_FLOAT:
        case OP_CMPL_DOUBLE:
        case OP_CMPG_DOUBLE:
        case OP_CMP_LONG:
        case OP_AGET:
        case OP_AGET_WIDE:
        case OP_AGET_OBJECT:
        case OP_AGET_BOOLEAN:
        case OP_AGET_BYTE:
        case OP_AGET_CHAR:
        case OP_AGET_SHORT:
        case OP_APUT:
        case OP_APUT_WIDE:
        case OP_APUT_OBJECT:
        case OP_APUT_BOOLEAN:
        case OP_APUT_BYTE:
        case OP_APUT_CHAR:
        case OP_APUT_SHORT:
        case OP_ADD_INT:
        case OP_SUB_INT:
        case OP_MUL_INT:
        case OP_DIV_INT:
        case OP_REM_INT:
        case OP_AND_INT:
        case OP_OR_INT:
        case OP_XOR_INT:
        case OP_SHL_INT:
        case OP_SHR_INT:
        case OP_USHR_INT:
        case OP_ADD_LONG:
        case OP_SUB_LONG:
        case OP_MUL_LONG:
        case OP_DIV_LONG:
        case OP_REM_LONG:
        case OP_AND_LONG:
        case OP_OR_LONG:
        case OP_XOR_LONG:
        case OP_SHL_LONG:
        case OP_SHR_LONG:
        case OP_USHR_LONG:
        case OP_ADD_FLOAT:
        case OP_SUB_FLOAT:
        case OP_MUL_FLOAT:
        case OP_DIV_FLOAT:
        case OP_REM_FLOAT:
        case OP_ADD_DOUBLE:
        case OP_SUB_DOUBLE:
        case OP_MUL_DOUBLE:
        case OP_DIV_DOUBLE:
        case OP_REM_DOUBLE:
            Format = OP_FORMAT_23x;
            break;
        case OP_CONST:
        case OP_CONST_WIDE_32:
            Format = OP_FORMAT_31i;
            break;
        case OP_CONST_STRING_JUMBO:
            Format = OP_FORMAT_31c;
            break;
        case OP_MOVE_16:
        case OP_MOVE_WIDE_16:
        case OP_MOVE_OBJECT_16:
            Format = OP_FORMAT_32x;
            break;
        case OP_FILLED_NEW_ARRAY:
        case OP_INVOKE_VIRTUAL:
        case OP_INVOKE_SUPER:
        case OP_INVOKE_DIRECT:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_INTERFACE:
            Format = OP_FORMAT_35c;
            break;
        case OP_FILLED_NEW_ARRAY_RANGE:
        case OP_INVOKE_VIRTUAL_RANGE:
        case OP_INVOKE_SUPER_RANGE:
        case OP_INVOKE_DIRECT_RANGE:
        case OP_INVOKE_STATIC_RANGE:
        case OP_INVOKE_INTERFACE_RANGE:
            Format = OP_FORMAT_3rc;
            break;
        case OP_CONST_WIDE:
            Format = OP_FORMAT_51l;
            break;

        /*
         * Optimized instructions.
         */
        case OP_THROW_VERIFICATION_ERROR:
            Format = OP_FORMAT_20bc;
            break;
        case OP_IGET_QUICK:
        case OP_IGET_WIDE_QUICK:
        case OP_IGET_OBJECT_QUICK:
        case OP_IPUT_QUICK:
        case OP_IPUT_WIDE_QUICK:
        case OP_IPUT_OBJECT_QUICK:
            Format = OP_FORMAT_22cs;
            break;
        case OP_INVOKE_VIRTUAL_QUICK:
        case OP_INVOKE_SUPER_QUICK:
            Format = OP_FORMAT_35ms;
            break;
        case OP_INVOKE_VIRTUAL_QUICK_RANGE:
        case OP_INVOKE_SUPER_QUICK_RANGE:
            Format = OP_FORMAT_3rms;
            break;
        case OP_EXECUTE_INLINE:
            Format = OP_FORMAT_3inline;
            break;
        case OP_INVOKE_DIRECT_EMPTY:
            Format = OP_FORMAT_35c;
            break;

        /* these should never appear */
        case OP_UNUSED_3E:
        case OP_UNUSED_3F:
        case OP_UNUSED_40:
        case OP_UNUSED_41:
        case OP_UNUSED_42:
        case OP_UNUSED_43:
        case OP_UNUSED_73:
        case OP_UNUSED_79:
        case OP_UNUSED_7A:
        case OP_UNUSED_E3:
        case OP_UNUSED_E4:
        case OP_UNUSED_E5:
        case OP_UNUSED_E6:
        case OP_UNUSED_E7:
        case OP_UNUSED_E8:
        case OP_UNUSED_E9:
        case OP_UNUSED_EA:
        case OP_UNUSED_EB:
        case OP_UNUSED_EC:
        case OP_UNUSED_EF:
        case OP_UNUSED_F1:
        case OP_UNUSED_FC:
        case OP_UNUSED_FD:
        case OP_UNUSED_FE:
        case OP_UNUSED_FF:
            Format = OP_FORMAT_UNKNOWN;
            break;

        /*
         * DO NOT add a "default" clause here.  Without it the compiler will
         * complain if an instruction is missing (which is desirable).
         */
        }

        OpcodesFormat[(CHAR)i] = Format;
    }
}

void cDexFile::CreateOpcodesFlagsTable()
{
    OpcodesFlags = new CHAR[256];
    UCHAR Flags;

    for (UINT i=0; i<256; i++) 
    {
        Flags = 0;

        switch ((CHAR)i) 
        {
        /* these don't affect the PC and can't cause an exception */
        case OP_NOP:
        case OP_MOVE:
        case OP_MOVE_FROM16:
        case OP_MOVE_16:
        case OP_MOVE_WIDE:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_WIDE_16:
        case OP_MOVE_OBJECT:
        case OP_MOVE_OBJECT_FROM16:
        case OP_MOVE_OBJECT_16:
        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
        case OP_MOVE_RESULT_OBJECT:
        case OP_MOVE_EXCEPTION:
        case OP_CONST_4:
        case OP_CONST_16:
        case OP_CONST:
        case OP_CONST_HIGH16:
        case OP_CONST_WIDE_16:
        case OP_CONST_WIDE_32:
        case OP_CONST_WIDE:
        case OP_CONST_WIDE_HIGH16:
        case OP_FILL_ARRAY_DATA:
        case OP_CMPL_FLOAT:
        case OP_CMPG_FLOAT:
        case OP_CMPL_DOUBLE:
        case OP_CMPG_DOUBLE:
        case OP_CMP_LONG:
        case OP_NEG_INT:
        case OP_NOT_INT:
        case OP_NEG_LONG:
        case OP_NOT_LONG:
        case OP_NEG_FLOAT:
        case OP_NEG_DOUBLE:
        case OP_INT_TO_LONG:
        case OP_INT_TO_FLOAT:
        case OP_INT_TO_DOUBLE:
        case OP_LONG_TO_INT:
        case OP_LONG_TO_FLOAT:
        case OP_LONG_TO_DOUBLE:
        case OP_FLOAT_TO_INT:
        case OP_FLOAT_TO_LONG:
        case OP_FLOAT_TO_DOUBLE:
        case OP_DOUBLE_TO_INT:
        case OP_DOUBLE_TO_LONG:
        case OP_DOUBLE_TO_FLOAT:
        case OP_INT_TO_BYTE:
        case OP_INT_TO_CHAR:
        case OP_INT_TO_SHORT:
        case OP_ADD_INT:
        case OP_SUB_INT:
        case OP_MUL_INT:
        case OP_AND_INT:
        case OP_OR_INT:
        case OP_XOR_INT:
        case OP_SHL_INT:
        case OP_SHR_INT:
        case OP_USHR_INT:
        case OP_ADD_LONG:
        case OP_SUB_LONG:
        case OP_MUL_LONG:
        case OP_AND_LONG:
        case OP_OR_LONG:
        case OP_XOR_LONG:
        case OP_SHL_LONG:
        case OP_SHR_LONG:
        case OP_USHR_LONG:
        case OP_ADD_FLOAT:
        case OP_SUB_FLOAT:
        case OP_MUL_FLOAT:
        case OP_DIV_FLOAT:
        case OP_REM_FLOAT:
        case OP_ADD_DOUBLE:
        case OP_SUB_DOUBLE:
        case OP_MUL_DOUBLE:
        case OP_DIV_DOUBLE:         // div by zero just returns NaN
        case OP_REM_DOUBLE:
        case OP_ADD_INT_2ADDR:
        case OP_SUB_INT_2ADDR:
        case OP_MUL_INT_2ADDR:
        case OP_AND_INT_2ADDR:
        case OP_OR_INT_2ADDR:
        case OP_XOR_INT_2ADDR:
        case OP_SHL_INT_2ADDR:
        case OP_SHR_INT_2ADDR:
        case OP_USHR_INT_2ADDR:
        case OP_ADD_LONG_2ADDR:
        case OP_SUB_LONG_2ADDR:
        case OP_MUL_LONG_2ADDR:
        case OP_AND_LONG_2ADDR:
        case OP_OR_LONG_2ADDR:
        case OP_XOR_LONG_2ADDR:
        case OP_SHL_LONG_2ADDR:
        case OP_SHR_LONG_2ADDR:
        case OP_USHR_LONG_2ADDR:
        case OP_ADD_FLOAT_2ADDR:
        case OP_SUB_FLOAT_2ADDR:
        case OP_MUL_FLOAT_2ADDR:
        case OP_DIV_FLOAT_2ADDR:
        case OP_REM_FLOAT_2ADDR:
        case OP_ADD_DOUBLE_2ADDR:
        case OP_SUB_DOUBLE_2ADDR:
        case OP_MUL_DOUBLE_2ADDR:
        case OP_DIV_DOUBLE_2ADDR:
        case OP_REM_DOUBLE_2ADDR:
        case OP_ADD_INT_LIT16:
        case OP_RSUB_INT:
        case OP_MUL_INT_LIT16:
        case OP_AND_INT_LIT16:
        case OP_OR_INT_LIT16:
        case OP_XOR_INT_LIT16:
        case OP_ADD_INT_LIT8:
        case OP_RSUB_INT_LIT8:
        case OP_MUL_INT_LIT8:
        case OP_AND_INT_LIT8:
        case OP_OR_INT_LIT8:
        case OP_XOR_INT_LIT8:
        case OP_SHL_INT_LIT8:
        case OP_SHR_INT_LIT8:
        case OP_USHR_INT_LIT8:
            Flags = OP_FLAG_CAN_CONTINUE;
            break;

        /* these don't affect the PC, but can cause exceptions */
        case OP_CONST_STRING:
        case OP_CONST_STRING_JUMBO:
        case OP_CONST_CLASS:
        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:
        case OP_CHECK_CAST:
        case OP_INSTANCE_OF:
        case OP_ARRAY_LENGTH:
        case OP_NEW_INSTANCE:
        case OP_NEW_ARRAY:
        case OP_FILLED_NEW_ARRAY:
        case OP_FILLED_NEW_ARRAY_RANGE:
        case OP_AGET:
        case OP_AGET_BOOLEAN:
        case OP_AGET_BYTE:
        case OP_AGET_CHAR:
        case OP_AGET_SHORT:
        case OP_AGET_WIDE:
        case OP_AGET_OBJECT:
        case OP_APUT:
        case OP_APUT_BOOLEAN:
        case OP_APUT_BYTE:
        case OP_APUT_CHAR:
        case OP_APUT_SHORT:
        case OP_APUT_WIDE:
        case OP_APUT_OBJECT:
        case OP_IGET:
        case OP_IGET_BOOLEAN:
        case OP_IGET_BYTE:
        case OP_IGET_CHAR:
        case OP_IGET_SHORT:
        case OP_IGET_WIDE:
        case OP_IGET_OBJECT:
        case OP_IPUT:
        case OP_IPUT_BOOLEAN:
        case OP_IPUT_BYTE:
        case OP_IPUT_CHAR:
        case OP_IPUT_SHORT:
        case OP_IPUT_WIDE:
        case OP_IPUT_OBJECT:
        case OP_SGET:
        case OP_SGET_BOOLEAN:
        case OP_SGET_BYTE:
        case OP_SGET_CHAR:
        case OP_SGET_SHORT:
        case OP_SGET_WIDE:
        case OP_SGET_OBJECT:
        case OP_SPUT:
        case OP_SPUT_BOOLEAN:
        case OP_SPUT_BYTE:
        case OP_SPUT_CHAR:
        case OP_SPUT_SHORT:
        case OP_SPUT_WIDE:
        case OP_SPUT_OBJECT:
        case OP_DIV_INT:
        case OP_REM_INT:
        case OP_DIV_LONG:
        case OP_REM_LONG:
        case OP_DIV_INT_2ADDR:
        case OP_REM_INT_2ADDR:
        case OP_DIV_LONG_2ADDR:
        case OP_REM_LONG_2ADDR:
        case OP_DIV_INT_LIT16:
        case OP_REM_INT_LIT16:
        case OP_DIV_INT_LIT8:
        case OP_REM_INT_LIT8:
            Flags = OP_FLAG_CAN_CONTINUE | OP_FLAG_CAN_THROW;
            break;

        case OP_INVOKE_VIRTUAL:
        case OP_INVOKE_VIRTUAL_RANGE:
        case OP_INVOKE_SUPER:
        case OP_INVOKE_SUPER_RANGE:
        case OP_INVOKE_DIRECT:
        case OP_INVOKE_DIRECT_RANGE:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_STATIC_RANGE:
        case OP_INVOKE_INTERFACE:
        case OP_INVOKE_INTERFACE_RANGE:
            Flags = OP_FLAG_CAN_CONTINUE | OP_FLAG_CAN_THROW | OP_FLAG_INVOKE;
            break;

        case OP_RETURN_VOID:
        case OP_RETURN:
        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:
            Flags = OP_FLAG_CAN_RETURN;
            break;

        case OP_THROW:
            Flags = OP_FLAG_CAN_THROW;
            break;

        /* unconditional branches */
        case OP_GOTO:
        case OP_GOTO_16:
        case OP_GOTO_32:
            Flags = OP_FLAG_CAN_BRANCH | OP_FLAG_UNCONDITIONAL;
            break;

        /* conditional branches */
        case OP_IF_EQ:
        case OP_IF_NE:
        case OP_IF_LT:
        case OP_IF_GE:
        case OP_IF_GT:
        case OP_IF_LE:
        case OP_IF_EQZ:
        case OP_IF_NEZ:
        case OP_IF_LTZ:
        case OP_IF_GEZ:
        case OP_IF_GTZ:
        case OP_IF_LEZ:
            Flags = OP_FLAG_CAN_BRANCH | OP_FLAG_CAN_CONTINUE;
            break;

        /* switch statements; if value not in switch, it continues */
        case OP_PACKED_SWITCH:
        case OP_SPARSE_SWITCH:
            Flags = OP_FLAG_CAN_SWITCH | OP_FLAG_CAN_CONTINUE;
            break;

        /* verifier/optimizer-generated instructions */
        case OP_THROW_VERIFICATION_ERROR:
            Flags = OP_FLAG_CAN_THROW;
            break;
        case OP_EXECUTE_INLINE:
            Flags = OP_FLAG_CAN_CONTINUE;
            break;
        case OP_IGET_QUICK:
        case OP_IGET_WIDE_QUICK:
        case OP_IGET_OBJECT_QUICK:
        case OP_IPUT_QUICK:
        case OP_IPUT_WIDE_QUICK:
        case OP_IPUT_OBJECT_QUICK:
            Flags = OP_FLAG_CAN_CONTINUE | OP_FLAG_CAN_THROW;
            break;

        case OP_INVOKE_VIRTUAL_QUICK:
        case OP_INVOKE_VIRTUAL_QUICK_RANGE:
        case OP_INVOKE_SUPER_QUICK:
        case OP_INVOKE_SUPER_QUICK_RANGE:
        case OP_INVOKE_DIRECT_EMPTY:
            Flags = OP_FLAG_CAN_CONTINUE | OP_FLAG_CAN_THROW | OP_FLAG_INVOKE;
            break;

        /* these should never appear */
        case OP_UNUSED_3E:
        case OP_UNUSED_3F:
        case OP_UNUSED_40:
        case OP_UNUSED_41:
        case OP_UNUSED_42:
        case OP_UNUSED_43:
        case OP_UNUSED_73:
        case OP_UNUSED_79:
        case OP_UNUSED_7A:
        case OP_UNUSED_E3:
        case OP_UNUSED_E4:
        case OP_UNUSED_E5:
        case OP_UNUSED_E6:
        case OP_UNUSED_E7:
        case OP_UNUSED_E8:
        case OP_UNUSED_E9:
        case OP_UNUSED_EA:
        case OP_UNUSED_EB:
        case OP_UNUSED_EC:
        case OP_UNUSED_EF:
        case OP_UNUSED_F1:
        case OP_UNUSED_FC:
        case OP_UNUSED_FD:
        case OP_UNUSED_FE:
        case OP_UNUSED_FF:
            break;

        /*
         * DO NOT add a "default" clause here.  Without it the compiler will
         * complain if an instruction is missing (which is desirable).
         */
        }

        OpcodesFlags[(CHAR)i] = Flags;
    }
}

void cDexFile::CreateOpcodesWidthsTable()
{
    OpcodesWidths = new CHAR[256];
    UINT Width;

    for (UINT i=0; i<256; i++)
    {
        Width = 0;

        switch ((CHAR)i) 
        {
        case OP_NOP:    /* note data for e.g. switch-* encoded "inside" a NOP */
        case OP_MOVE:
        case OP_MOVE_WIDE:
        case OP_MOVE_OBJECT:
        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
        case OP_MOVE_RESULT_OBJECT:
        case OP_MOVE_EXCEPTION:
        case OP_RETURN_VOID:
        case OP_RETURN:
        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:
        case OP_CONST_4:
        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:
        case OP_ARRAY_LENGTH:
        case OP_THROW:
        case OP_GOTO:
        case OP_NEG_INT:
        case OP_NOT_INT:
        case OP_NEG_LONG:
        case OP_NOT_LONG:
        case OP_NEG_FLOAT:
        case OP_NEG_DOUBLE:
        case OP_INT_TO_LONG:
        case OP_INT_TO_FLOAT:
        case OP_INT_TO_DOUBLE:
        case OP_LONG_TO_INT:
        case OP_LONG_TO_FLOAT:
        case OP_LONG_TO_DOUBLE:
        case OP_FLOAT_TO_INT:
        case OP_FLOAT_TO_LONG:
        case OP_FLOAT_TO_DOUBLE:
        case OP_DOUBLE_TO_INT:
        case OP_DOUBLE_TO_LONG:
        case OP_DOUBLE_TO_FLOAT:
        case OP_INT_TO_BYTE:
        case OP_INT_TO_CHAR:
        case OP_INT_TO_SHORT:
        case OP_ADD_INT_2ADDR:
        case OP_SUB_INT_2ADDR:
        case OP_MUL_INT_2ADDR:
        case OP_DIV_INT_2ADDR:
        case OP_REM_INT_2ADDR:
        case OP_AND_INT_2ADDR:
        case OP_OR_INT_2ADDR:
        case OP_XOR_INT_2ADDR:
        case OP_SHL_INT_2ADDR:
        case OP_SHR_INT_2ADDR:
        case OP_USHR_INT_2ADDR:
        case OP_ADD_LONG_2ADDR:
        case OP_SUB_LONG_2ADDR:
        case OP_MUL_LONG_2ADDR:
        case OP_DIV_LONG_2ADDR:
        case OP_REM_LONG_2ADDR:
        case OP_AND_LONG_2ADDR:
        case OP_OR_LONG_2ADDR:
        case OP_XOR_LONG_2ADDR:
        case OP_SHL_LONG_2ADDR:
        case OP_SHR_LONG_2ADDR:
        case OP_USHR_LONG_2ADDR:
        case OP_ADD_FLOAT_2ADDR:
        case OP_SUB_FLOAT_2ADDR:
        case OP_MUL_FLOAT_2ADDR:
        case OP_DIV_FLOAT_2ADDR:
        case OP_REM_FLOAT_2ADDR:
        case OP_ADD_DOUBLE_2ADDR:
        case OP_SUB_DOUBLE_2ADDR:
        case OP_MUL_DOUBLE_2ADDR:
        case OP_DIV_DOUBLE_2ADDR:
        case OP_REM_DOUBLE_2ADDR:
            Width = 1;
            break;

        case OP_MOVE_FROM16:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_OBJECT_FROM16:
        case OP_CONST_16:
        case OP_CONST_HIGH16:
        case OP_CONST_WIDE_16:
        case OP_CONST_WIDE_HIGH16:
        case OP_CONST_STRING:
        case OP_CONST_CLASS:
        case OP_CHECK_CAST:
        case OP_INSTANCE_OF:
        case OP_NEW_INSTANCE:
        case OP_NEW_ARRAY:
        case OP_CMPL_FLOAT:
        case OP_CMPG_FLOAT:
        case OP_CMPL_DOUBLE:
        case OP_CMPG_DOUBLE:
        case OP_CMP_LONG:
        case OP_GOTO_16:
        case OP_IF_EQ:
        case OP_IF_NE:
        case OP_IF_LT:
        case OP_IF_GE:
        case OP_IF_GT:
        case OP_IF_LE:
        case OP_IF_EQZ:
        case OP_IF_NEZ:
        case OP_IF_LTZ:
        case OP_IF_GEZ:
        case OP_IF_GTZ:
        case OP_IF_LEZ:
        case OP_AGET:
        case OP_AGET_WIDE:
        case OP_AGET_OBJECT:
        case OP_AGET_BOOLEAN:
        case OP_AGET_BYTE:
        case OP_AGET_CHAR:
        case OP_AGET_SHORT:
        case OP_APUT:
        case OP_APUT_WIDE:
        case OP_APUT_OBJECT:
        case OP_APUT_BOOLEAN:
        case OP_APUT_BYTE:
        case OP_APUT_CHAR:
        case OP_APUT_SHORT:
        case OP_IGET:
        case OP_IGET_WIDE:
        case OP_IGET_OBJECT:
        case OP_IGET_BOOLEAN:
        case OP_IGET_BYTE:
        case OP_IGET_CHAR:
        case OP_IGET_SHORT:
        case OP_IPUT:
        case OP_IPUT_WIDE:
        case OP_IPUT_OBJECT:
        case OP_IPUT_BOOLEAN:
        case OP_IPUT_BYTE:
        case OP_IPUT_CHAR:
        case OP_IPUT_SHORT:
        case OP_SGET:
        case OP_SGET_WIDE:
        case OP_SGET_OBJECT:
        case OP_SGET_BOOLEAN:
        case OP_SGET_BYTE:
        case OP_SGET_CHAR:
        case OP_SGET_SHORT:
        case OP_SPUT:
        case OP_SPUT_WIDE:
        case OP_SPUT_OBJECT:
        case OP_SPUT_BOOLEAN:
        case OP_SPUT_BYTE:
        case OP_SPUT_CHAR:
        case OP_SPUT_SHORT:
        case OP_ADD_INT:
        case OP_SUB_INT:
        case OP_MUL_INT:
        case OP_DIV_INT:
        case OP_REM_INT:
        case OP_AND_INT:
        case OP_OR_INT:
        case OP_XOR_INT:
        case OP_SHL_INT:
        case OP_SHR_INT:
        case OP_USHR_INT:
        case OP_ADD_LONG:
        case OP_SUB_LONG:
        case OP_MUL_LONG:
        case OP_DIV_LONG:
        case OP_REM_LONG:
        case OP_AND_LONG:
        case OP_OR_LONG:
        case OP_XOR_LONG:
        case OP_SHL_LONG:
        case OP_SHR_LONG:
        case OP_USHR_LONG:
        case OP_ADD_FLOAT:
        case OP_SUB_FLOAT:
        case OP_MUL_FLOAT:
        case OP_DIV_FLOAT:
        case OP_REM_FLOAT:
        case OP_ADD_DOUBLE:
        case OP_SUB_DOUBLE:
        case OP_MUL_DOUBLE:
        case OP_DIV_DOUBLE:
        case OP_REM_DOUBLE:
        case OP_ADD_INT_LIT16:
        case OP_RSUB_INT:
        case OP_MUL_INT_LIT16:
        case OP_DIV_INT_LIT16:
        case OP_REM_INT_LIT16:
        case OP_AND_INT_LIT16:
        case OP_OR_INT_LIT16:
        case OP_XOR_INT_LIT16:
        case OP_ADD_INT_LIT8:
        case OP_RSUB_INT_LIT8:
        case OP_MUL_INT_LIT8:
        case OP_DIV_INT_LIT8:
        case OP_REM_INT_LIT8:
        case OP_AND_INT_LIT8:
        case OP_OR_INT_LIT8:
        case OP_XOR_INT_LIT8:
        case OP_SHL_INT_LIT8:
        case OP_SHR_INT_LIT8:
        case OP_USHR_INT_LIT8:
            Width = 2;
            break;

        case OP_MOVE_16:
        case OP_MOVE_WIDE_16:
        case OP_MOVE_OBJECT_16:
        case OP_CONST:
        case OP_CONST_WIDE_32:
        case OP_CONST_STRING_JUMBO:
        case OP_GOTO_32:
        case OP_FILLED_NEW_ARRAY:
        case OP_FILLED_NEW_ARRAY_RANGE:
        case OP_FILL_ARRAY_DATA:
        case OP_PACKED_SWITCH:
        case OP_SPARSE_SWITCH:
        case OP_INVOKE_VIRTUAL:
        case OP_INVOKE_SUPER:
        case OP_INVOKE_DIRECT:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_INTERFACE:
        case OP_INVOKE_VIRTUAL_RANGE:
        case OP_INVOKE_SUPER_RANGE:
        case OP_INVOKE_DIRECT_RANGE:
        case OP_INVOKE_STATIC_RANGE:
        case OP_INVOKE_INTERFACE_RANGE:
            Width = 3;
            break;

        case OP_CONST_WIDE:
            Width = 5;
            break;

        /*
         * Optimized instructions. Rreturn negative size values for these
         * to distinguish them.
         */
        case OP_IGET_QUICK:
        case OP_IGET_WIDE_QUICK:
        case OP_IGET_OBJECT_QUICK:
        case OP_IPUT_QUICK:
        case OP_IPUT_WIDE_QUICK:
        case OP_IPUT_OBJECT_QUICK:
        case OP_THROW_VERIFICATION_ERROR:
            Width = -2;
            break;
        case OP_INVOKE_VIRTUAL_QUICK:
        case OP_INVOKE_VIRTUAL_QUICK_RANGE:
        case OP_INVOKE_SUPER_QUICK:
        case OP_INVOKE_SUPER_QUICK_RANGE:
        case OP_EXECUTE_INLINE:
        case OP_INVOKE_DIRECT_EMPTY:
            Width = -3;
            break;

        /* these should never appear */
        case OP_UNUSED_3E:
        case OP_UNUSED_3F:
        case OP_UNUSED_40:
        case OP_UNUSED_41:
        case OP_UNUSED_42:
        case OP_UNUSED_43:
        case OP_UNUSED_73:
        case OP_UNUSED_79:
        case OP_UNUSED_7A:
        case OP_UNUSED_E3:
        case OP_UNUSED_E4:
        case OP_UNUSED_E5:
        case OP_UNUSED_E6:
        case OP_UNUSED_E7:
        case OP_UNUSED_E8:
        case OP_UNUSED_E9:
        case OP_UNUSED_EA:
        case OP_UNUSED_EB:
        case OP_UNUSED_EC:
        case OP_UNUSED_EF:
        case OP_UNUSED_F1:
        case OP_UNUSED_FC:
        case OP_UNUSED_FD:
        case OP_UNUSED_FE:
        case OP_UNUSED_FF:
            break;

        /*
         * DO NOT add a "default" clause here.  Without it the compiler will
         * complain if an instruction is missing (which is desirable).
         */
        }

        OpcodesWidths[(CHAR)i] = Width;
    }
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
