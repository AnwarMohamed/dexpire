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

cDexFile::cDexFile(CHAR* Filename): 
    cFile(Filename)
{
    OpcodeCounter = 0;
    isReady = (BaseAddress && FileLength >= sizeof(DEX_HEADER) && DumpDex());
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
    (*Class).ClassData->StaticFieldsSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
    (*Class).ClassData->InstanceFieldsSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
    (*Class).ClassData->DirectMethodsSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);    
    (*Class).ClassData->VirtualMethodsSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
    (*Class).ClassData->InterfacesSize = 
        DexClassDefs[ClassIndex].InterfacesOff? *(UINT*)(BaseAddress + DexClassDefs[ClassIndex].InterfacesOff): 0;
}

void cDexFile::AllocateClassData(
    UINT ClassIndex, 
    DEX_CLASS_STRUCTURE* Class
    )
{
    /* Allocating Static Fields */
    if ((*Class).ClassData->StaticFieldsSize)
    {
        (*Class).ClassData->StaticFields = new CLASS_FIELD[(*Class).ClassData->StaticFieldsSize];
        ZERO((*Class).ClassData->StaticFields, (*Class).ClassData->StaticFieldsSize * sizeof(CLASS_FIELD));
    }

    /* Allocating Instance Fields */
    if ((*Class).ClassData->InstanceFieldsSize)
    {
        (*Class).ClassData->InstanceFields = new CLASS_FIELD[(*Class).ClassData->InstanceFieldsSize];
        ZERO((*Class).ClassData->InstanceFields, (*Class).ClassData->InstanceFieldsSize * sizeof(CLASS_FIELD));
    }

    /* Allocating Direct Methods */
    if ((*Class).ClassData->DirectMethodsSize)
    {
        (*Class).ClassData->DirectMethods = new CLASS_METHOD[(*Class).ClassData->DirectMethodsSize];
        ZERO((*Class).ClassData->DirectMethods, (*Class).ClassData->DirectMethodsSize * sizeof(CLASS_METHOD));
    }

    /* Allocating Virtual Methods */
    if ((*Class).ClassData->VirtualMethodsSize)
    {
        (*Class).ClassData->VirtualMethods = new CLASS_METHOD[(*Class).ClassData->VirtualMethodsSize];
        ZERO((*Class).ClassData->VirtualMethods, (*Class).ClassData->VirtualMethodsSize * sizeof(CLASS_METHOD));
    }        

    /* Allocating Interfaces */
    if ((*Class).ClassData->InterfacesSize)
    {
        (*Class).ClassData->Interfaces = new UCHAR*[(*Class).ClassData->InterfacesSize];
        ZERO((*Class).ClassData->Interfaces, (*Class).ClassData->InterfacesSize * sizeof(UCHAR*));
    }
}

void cDexFile::DumpFieldByIndex(
    UINT FieldIndex, 
    CLASS_FIELD* Field, 
    UCHAR** Buffer
    )
{
    (*Field).Type = StringItems[DexTypeIds[ DexFieldIds[FieldIndex].TypeIdex ].StringIndex].Data;
    (*Field).Name = StringItems[DexFieldIds[FieldIndex].StringIndex].Data;
    (*Field).AccessFlags = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
} 

void cDexFile::DumpInterfaceByIndex(
    UINT ClassIndex, 
    UINT InterfaceIndex, 
    UCHAR** Interface
    )
{
    (*Interface) = StringItems[DexTypeIds[ 
        ((USHORT*)(BaseAddress+DexClassDefs[ClassIndex].InterfacesOff + sizeof(UINT)))[InterfaceIndex] 
    ].StringIndex].Data;
}

void cDexFile::DumpFieldsValues(
    UINT Offset,
    CLASS_DATA* ClassData
    )
{
    if (!Offset) return;

    UCHAR* Ptr = (UCHAR*)(BaseAddress + Offset);
    UINT ValuesSize = ReadUnsignedLeb128((CONST UCHAR**)&Ptr);

    UCHAR ValueType, ValueSize;
    CHAR* Temp = new CHAR[TEMP_STRING_SIZE];

    for (UINT i=0; i<ValuesSize; i++)
    {
        ValueType = *Ptr++;
        ValueSize = (ValueType >> 5) +1;

        ZERO(Temp,TEMP_STRING_SIZE);

        switch(ValueType)
        {    
        case VALUE_BYTE:
            _itoa_s(*(CHAR*)Ptr, Temp, TEMP_STRING_SIZE, 10);
            Ptr += 1;
            break;
        case VALUE_SHORT:
            _itoa_s((*(SHORT*)Ptr) << ((2-ValueSize)*8) >> ((2-ValueSize)*8), Temp, TEMP_STRING_SIZE, 10);
            Ptr += ValueSize;
            break;
        case VALUE_CHAR:
            _itoa_s((*(UCHAR*)Ptr) << ((2-ValueSize)*8) >> ((2-ValueSize)*8) , Temp, TEMP_STRING_SIZE, 10);
            Ptr += ValueSize;
            break;
        case VALUE_INT:
            _itoa_s((*(INT*)Ptr) << ((4-ValueSize)*8) >> ((4-ValueSize)*8), Temp, TEMP_STRING_SIZE, 10);
            Ptr += ValueSize;
            break;
        case VALUE_LONG:
            _ltoa_s((*(LONG*)Ptr) << ((8-ValueSize)*8) >> ((8-ValueSize)*8), Temp, TEMP_STRING_SIZE, 10);
            Ptr += ValueSize;
            break;
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
            Ptr += ValueSize;
            break;
        case VALUE_STRING:
            sprintf_s(Temp, TEMP_STRING_SIZE, "\"%s\"", StringItems[(*(UINT*)Ptr) << ((4-ValueSize)*8) >> ((4-ValueSize)*8)].Data);
            Ptr  += ValueSize;
            break;
        case VALUE_TYPE:
        case VALUE_FIELD:
        case VALUE_METHOD:
        case VALUE_ENUM:
            Ptr += ValueSize;
            break;
        case VALUE_ARRAY:
        case VALUE_ANNOTATION:
        case VALUE_NULL:
            sprintf_s(Temp, TEMP_STRING_SIZE, "null");
            break;
        case VALUE_BOOLEAN:
        case VALUE_SENTINEL:
            break;
        }

        ClassData->StaticFields[i].Value = new UCHAR[strlen(Temp)+1];
        memcpy(ClassData->StaticFields[i].Value, Temp, strlen(Temp)+1);
    }

    delete Temp;
}

BOOL cDexFile::DumpDex()
{
    UCHAR* Ptr;
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
    ZERO(StringItems, nStringItems * sizeof(DEX_STRING_ITEM));

    for (UINT i=0; i<nStringIDs; i++)
    {
        Ptr = (UCHAR*)BaseAddress + DexStringIds[i].StringDataOff;
        StringItems[i].StringSize = ReadUnsignedLeb128((CONST UCHAR**)&Ptr);
        StringItems[i].Data = Ptr;
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
    ZERO(DexClasses, nClasses * sizeof(DEX_CLASS_STRUCTURE));
    DexClassDefs = (DEX_CLASS_DEF*)(BaseAddress + DexHeader->ClassDefsOff);

    /* Opcode Parsing Tables */
    CreateOpcodesWidthsTable();
    CreateOpcodesFlagsTable();
    CreateOpcodesFormatTable();

    for (UINT i=0; i<nClasses; i++)
    {
        DumpClassInfo(i, &DexClasses[i]);

        if (DexClassDefs[i].ClassDataOff != NULL)
        {
            DexClasses[i].ClassData = new CLASS_DATA;
            ZERO(DexClasses[i].ClassData, sizeof(CLASS_DATA));
            DexClassData = (DEX_CLASS_DATA*)(BaseAddress + DexClassDefs[i].ClassDataOff);

            Ptr = (UCHAR*)DexClassData;
            DumpClassDataInfo(i, &DexClasses[i], &Ptr);
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
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    &DexClasses[i].ClassData->StaticFields[j], 
                    &Ptr);

            /* Parsing Static Fields Vales */
            DumpFieldsValues(DexClassDefs[i].StaticValuesOff, DexClasses[i].ClassData);

            /* Parsing Instance Fields */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->InstanceFieldsSize; j++)
                DumpFieldByIndex(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    &DexClasses[i].ClassData->InstanceFields[j], 
                    &Ptr);

            /* Parsing Direct Methods */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->DirectMethodsSize; j++)
                DumpMethodById(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    &DexClasses[i].ClassData->DirectMethods[j],
                    &Ptr);

            /* Parsing Virtual Methods */
            CurIndex = 0;
            for (UINT j=0; j<DexClasses[i].ClassData->VirtualMethodsSize; j++)
                DumpMethodById(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    &DexClasses[i].ClassData->VirtualMethods[j],
                    &Ptr);

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
        DexClass->ClassData->Annotations = new CLASS_ANNOTATION[DexClass->ClassData->AnnotationsSize];
        ZERO(DexClass->ClassData->Annotations, DexClass->ClassData->AnnotationsSize * sizeof(CLASS_ANNOTATION));

        for (UINT i=0; i<ClassAnnotations->Size; i++)
        {
            ClassAnnotationItem = (DEX_ANNOTATION_ITEM*)(BaseAddress + ClassAnnotations->Entries[i].AnnotationOff);
            Ptr = (UCHAR*)ClassAnnotationItem->Encoded;

            DexClass->ClassData->Annotations[i].Type = StringItems[DexTypeIds[ReadUnsignedLeb128((CONST UCHAR**)&Ptr)].StringIndex].Data;
            DexClass->ClassData->Annotations[i].Visibility = ClassAnnotationItem->Visibility;
            DexClass->ClassData->Annotations[i].ElementsSize = ReadUnsignedLeb128((CONST UCHAR**)&Ptr);

            if (DexClass->ClassData->Annotations[i].ElementsSize)
            {
                DexClass->ClassData->Annotations[i].Elements = 
                    new CLASS_ANNOTATION_ELEMENT[DexClass->ClassData->Annotations[i].ElementsSize];
                ZERO(DexClass->ClassData->Annotations[i].Elements,
                    DexClass->ClassData->Annotations[i].ElementsSize * sizeof(CLASS_ANNOTATION_ELEMENT));
            }

            for (UINT j=0; j<DexClass->ClassData->Annotations[i].ElementsSize; j++)
            {
                DexClass->ClassData->Annotations[i].Elements[j].Name = StringItems[ReadUnsignedLeb128((CONST UCHAR**)&Ptr)].Data;
                DumpAnnotationElementValue(&DexClass->ClassData->Annotations[i].Elements[j], &Ptr);
            }
        }
    }

    if (Annotations->FieldsSize)
    {
        DEX_FIELD_ANNOTATION* FieldAnnotations = (DEX_FIELD_ANNOTATION*)Annotations->Ptr;
        for (UINT i=0; i<Annotations->FieldsSize; i++);
    }
}

void cDexFile::DumpAnnotationElementValue(
    CLASS_ANNOTATION_ELEMENT* Element, 
    UCHAR** Ptr
    )
{
    Element->ValueType = *((*Ptr)++);
    Element->ValueSize = (Element->ValueType >> 5) +1;
    Element->Value = *Ptr;
    Ptr+= Element->ValueSize;
}

void cDexFile::DumpMethodById(
    UINT MethodIndex,
    CLASS_METHOD* Method,
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
    (*Method).AccessFlags = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

    /* Parse Method Code */
    DumpMethodCode(
        (DEX_CODE*)(BaseAddress + ReadUnsignedLeb128((CONST UCHAR**)Buffer)),
        Method);
}

void cDexFile::DumpMethodCode(
    DEX_CODE* CodeAreaDef,
    CLASS_METHOD* Method
    )
{
    if ((DWORD)CodeAreaDef == BaseAddress)
        (*Method).CodeArea = NULL;
    else
    {
        (*Method).CodeArea = new CLASS_CODE;
        ZERO((*Method).CodeArea, sizeof(CLASS_CODE));

        DumpMethodCodeInfo(
            (*Method).CodeArea,
            CodeAreaDef);
 
        /* Debug Info */
        TempBuffer = (UCHAR*)(BaseAddress + (*CodeAreaDef).DebugInfoOff);
        DumpMethodDebugInfo(
            Method,
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
    CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef    
    )
{
    (*CodeArea).InsSize = (*CodeAreaDef).InsSize;
    (*CodeArea).RegistersSize = (*CodeAreaDef).RegistersSize;
    (*CodeArea).OutsSize = (*CodeAreaDef).OutsSize;
    (*CodeArea).TriesSize = (*CodeAreaDef).TriesSize;
    (*CodeArea).InstBufferSize = (*CodeAreaDef).InstructionsSize;
}

void cDexFile::DumpMethodLocals(
    CLASS_METHOD* Method, 
    UCHAR** Buffer)
{
    Method->CodeArea->Locals = new map<UINT, CLASS_CODE_LOCAL*>();
    USHORT LocalsIndex = Method->CodeArea->RegistersSize - Method->CodeArea->InsSize;

    CLASS_CODE_LOCAL* Local;
    if (!(Method->AccessFlags & ACC_STATIC))
    {
        Local = new CLASS_CODE_LOCAL;
        Local->Name = "this";
        Local->Type = NULL;
        (*Method->CodeArea->Locals)[LocalsIndex++] = Local;
    }

    UINT LocalsSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
    UINT LocalNameIndex, TypePtr = 0;
    for (UINT i=0; i<LocalsSize; i++)
    {
        LocalNameIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer) - 1;
        
        Local = new CLASS_CODE_LOCAL;
        (*Method->CodeArea->Locals)[LocalsIndex] = Local;

        if(LocalNameIndex == NO_INDEX)
            Local->Name = NULL;
        else
            Local->Name = (CHAR*)StringItems[LocalNameIndex].Data;

        switch (Method->Type[TypePtr]) {
            case 'D':
            case 'J':
                LocalsIndex += 2;
                break;
            default:
                LocalsIndex += 1;
                break;
        }

        switch(Method->Type[TypePtr])
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
            Local->Type = cDexString::GetTypeDescription((CHAR*)&Method->Type[TypePtr]);
            TypePtr++;  break;
        case 'L':
        case '[':
            Local->Type = cDexString::GetTypeDescription((CHAR*)&Method->Type[TypePtr]);
            if (Method->Type[TypePtr] == 'L')
                TypePtr += strlen(Local->Type) + 2;   
            else
                TypePtr += cDexString::GetArrayTypeSize((CHAR*)Method->Type + TypePtr);
            break;
        }
    }
}

void cDexFile::DumpMethodDebugInfo(
    CLASS_METHOD* Method,
    CLASS_CODE_DEBUG_INFO* DebugInfo,
    UCHAR** Buffer
    )
{
    /* -------------------------------------------------------- */
    UINT line = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
    UINT insnsSize = Method->CodeArea->InstBufferSize/2;
    UINT address = 0;

    DumpMethodLocals(Method, Buffer);

    if (Method->CodeArea->RegistersSize)
    {
        Method->CodeArea->Registers = new CLASS_CODE_REGISTER[Method->CodeArea->RegistersSize];
        ZERO(Method->CodeArea->Registers, Method->CodeArea->RegistersSize* sizeof(CLASS_CODE_REGISTER));
    }
     

    UINT RegisterNumber;
    UINT NameIndex;
    UINT TypeIndex;
    UINT SigIndex;

    UCHAR Opcode;
    while(TRUE)
    {
        Opcode = *(*Buffer)++;
        OpcodeCounter++;
        switch (Opcode) 
        {
        case DBG_END_SEQUENCE:
            return;

        case DBG_ADVANCE_PC:
            address += ReadUnsignedLeb128((CONST UCHAR**)Buffer);
            break;

        case DBG_ADVANCE_LINE:
            line += ReadSignedLeb128((CONST UCHAR**)Buffer);
            break;

        case DBG_START_LOCAL:
        case DBG_START_LOCAL_EXTENDED:
            InsertDebugPosition(Method->CodeArea, line, address);
            RegisterNumber = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
            if (RegisterNumber > Method->CodeArea->RegistersSize)
                return;
            
            /*
            NameIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1;
            if (NameIndex != NO_INDEX)
                CodeArea->Locals[RegisterNumber].Name = (CHAR*)StringItems[NameIndex].Data;

            TypeIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1;
            if (TypeIndex != NO_INDEX)
                CodeArea->Locals[RegisterNumber].Type = (CHAR*)StringItems[TypeIndex].Data;

            if (Opcode == DBG_START_LOCAL_EXTENDED)
            {
                SigIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1;
                if (SigIndex != NO_INDEX)
                    CodeArea->Locals[RegisterNumber].Signature = (CHAR*)StringItems[SigIndex].Data;
            }
            
            CodeArea->Locals[RegisterNumber].OffsetStart = address;
            */

            break;

        case DBG_END_LOCAL:
            InsertDebugPosition(Method->CodeArea, line, address);
            RegisterNumber = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
            break;
        case DBG_RESTART_LOCAL:
            RegisterNumber = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
            break;

        case DBG_SET_PROLOGUE_END:
        case DBG_SET_EPILOGUE_BEGIN:
            break;

        case DBG_SET_FILE:
            //(*DebugInfo).Registers.SourceFile = (CHAR*)StringItems[ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1].Data;
            break;
        default:
            {
                int new_opcode = Opcode - DBG_FIRST_SPECIAL;                
                line += DBG_LINE_BASE + (new_opcode % DBG_LINE_RANGE);
                address += new_opcode / DBG_LINE_RANGE;
                //printf("line=%d address=0x%04x\n", line, address);
                InsertDebugPosition(Method->CodeArea, line, address);
            }
        };
    }
}

void cDexFile::InsertDebugPosition(
    CLASS_CODE* CodeArea,
    UINT Line,
    USHORT Offset
    )
{
    CodeArea->DebugInfo.Positions = (CLASS_CODE_DEBUG_POSITION**)realloc
        (CodeArea->DebugInfo.Positions, ++CodeArea->DebugInfo.PositionsSize *sizeof(CLASS_CODE_DEBUG_POSITION*));
    CodeArea->DebugInfo.Positions[CodeArea->DebugInfo.PositionsSize-1] = new CLASS_CODE_DEBUG_POSITION;
    CodeArea->DebugInfo.Positions[CodeArea->DebugInfo.PositionsSize-1]->Line = Line;
    CodeArea->DebugInfo.Positions[CodeArea->DebugInfo.PositionsSize-1]->Offset = Offset;
}

void cDexFile::DumpMethodInstructions(
    CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef
    )
{
    if ((*CodeAreaDef).InstructionsSize > 0)
    {
        
        (*CodeArea).Instructions = (CLASS_CODE_INSTRUCTION**)malloc( (*CodeArea).InstructionsSize * sizeof(CLASS_CODE_INSTRUCTION*));
        (*CodeArea).Offset = (DWORD)(CodeAreaDef) - BaseAddress;

        CHAR Opcode;
        for (UINT i=0; i<(*CodeAreaDef).InstructionsSize*2;)
        {
            Opcode = ((CHAR*)((*CodeAreaDef).Instructions))[i];

            (*CodeArea).Instructions = (CLASS_CODE_INSTRUCTION**)
                realloc((*CodeArea).Instructions, ++(*CodeArea).InstructionsSize * sizeof(CLASS_CODE_INSTRUCTION*));

            (*CodeArea).Instructions[(*CodeArea).InstructionsSize-1] = DecodeOpcode(((UCHAR*)((*CodeAreaDef).Instructions)) + i);
            (*CodeArea).Instructions[(*CodeArea).InstructionsSize-1]->Offset = (DWORD)(CodeAreaDef->Instructions) - BaseAddress + i/2;

            i+= (*CodeArea).Instructions[(*CodeArea).InstructionsSize-1]->BytesSize; 
        }
    }
    else
        (*CodeArea).Instructions = NULL;
}

CLASS_CODE_INSTRUCTION* cDexFile::DecodeOpcode(
    UCHAR* Opcode
    )
{
    CLASS_CODE_INSTRUCTION* Decoded = new CLASS_CODE_INSTRUCTION;
    ZERO(Decoded, sizeof(CLASS_CODE_INSTRUCTION));

    Decoded->Opcode = (UCHAR*)OpcodesStrings[Opcode[0]];
    Decoded->Format = (UCHAR*)OpcodesFormatStrings[OpcodesFormat[Opcode[0]]];
    Decoded->Bytes = (UCHAR*)Opcode;
    Decoded->BytesSize = 1;

    CHAR* Temp = new CHAR[TEMP_STRING_SIZE];

    switch(OpcodesFormat[(UCHAR)Opcode[0]])
    {
    case OP_FORMAT_UNKNOWN:
        sprintf_s(Temp, TEMP_STRING_SIZE, "<unknown>");
        Decoded->BytesSize += 1;
        break;

    case OP_FORMAT_10x:        // op
        if (*Opcode == OP_NOP) 
            switch(Opcode[0] | (Opcode[1] << 8))
            {
            case kPackedSwitchSignature:
                sprintf_s(Temp, TEMP_STRING_SIZE, "packed-switch-data");
                Decoded->BytesSize = ((*(USHORT*)(Opcode+2))*2)+4;
                Decoded->BytesSize*=4;
                break;
            case kSparseSwitchSignature:
                sprintf_s(Temp, TEMP_STRING_SIZE, "sparse-switch-data");
                Decoded->BytesSize = ((*(USHORT*)(Opcode+2)) * 4) + 2;
                Decoded->BytesSize*=4;
                break;
            case kArrayDataSignature:
                sprintf_s(Temp, TEMP_STRING_SIZE, "array-data");
                Decoded->BytesSize = ((*(USHORT*)(Opcode+2)) * (*(UINT*)(Opcode+4)) + 1) / 2 + 4;
                Decoded->BytesSize*=4;
                break;
            default:
                sprintf_s(Temp, TEMP_STRING_SIZE, "nop");
                Decoded->BytesSize += 1;
            }
        else
        {
            sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
            Decoded->BytesSize += 1;
        }
        break;

    case OP_FORMAT_12x:        // op vA, vB
        Decoded->vA = Opcode[1] & 0x0F;
        Decoded->vB = Opcode[1] >> 4;
        Decoded->BytesSize += 1;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_11n:        // op vA, #+B
        Decoded->vA = Opcode[1] & 0x0F;
        Decoded->vB = (CHAR)(Opcode[1] >> 4);
        Decoded->BytesSize += 1;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode, 
            Decoded->vA,
            (CHAR)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_11x:        // op vAA
        Decoded->vA = Opcode[1];
        Decoded->BytesSize += 1;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d", 
            Decoded->Opcode, 
            Decoded->vA);
        break;

    case OP_FORMAT_10t:        // op +AA
        Decoded->vA = (CHAR)Opcode[1];
        Decoded->BytesSize += 1;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s %s%d", 
            Decoded->Opcode, 
            (CHAR)Decoded->vA>=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_20bc:       // op AA, thing@BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s %d, %s@%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            "thing", 
            Decoded->vB);
        break;

    case OP_FORMAT_20t:        // op +AAAA
        Decoded->vA = *(SHORT*)&Opcode[1];
        Decoded->BytesSize += 1;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s %s%d", 
            Decoded->Opcode, 
            (SHORT)Decoded->vA>=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_22x:        // op vAA, vBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB);
        break;

    case OP_FORMAT_21t:        // op vAA, +BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, %s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21s:        // op vAA, #+BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21h:        // op vAA, #+BBBB00000[00000000]
        Decoded->vA = Opcode[1];
        Decoded->vB = *(SHORT*)&Opcode[2];
        Decoded->BytesSize += 3;

        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21c:        // op vAA, thing@BBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[2];
        Decoded->BytesSize += 3;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, %s@%d", 
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
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d, v%d", 
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
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d, #%s%d", 
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
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d, %s%d", 
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
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d, #%s%d", 
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
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d, %s@%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            Opcode[0] >= OP_IGET && Opcode[0] <= OP_IPUT_SHORT? "field" :
            "class",
            Decoded->vC);
        break;

    case OP_FORMAT_22cs:       // [opt] op vA, vB, field offset CCCC
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_32x:        // op vAAAA, vBBBB
        Decoded->vA = *(USHORT*)&Opcode[1];
        Decoded->vB = *(USHORT*)&Opcode[3];
        Decoded->BytesSize += 5;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, v%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_30t:        // op +AAAAAAAA
        Decoded->vA = *(INT*)&Opcode[1];
        Decoded->BytesSize += 5;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s %s%d", 
            Decoded->Opcode,
            (INT)Decoded->vA<=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_31t:        // op vAA, +BBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(INT*)&Opcode[2];
        Decoded->BytesSize += 5;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, %s%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT)Decoded->vB<=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_31i:        // op vAA, #+BBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(INT*)&Opcode[2];
        Decoded->BytesSize += 5;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT)Decoded->vB<=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_31c:        // op vAA, thing@BBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB = *(UINT*)&Opcode[2];
        Decoded->BytesSize += 5;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, string@%d", 
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
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s {", Decoded->Opcode);
        
        if (Decoded->vA)
            sprintf_s(Temp+strlen(Temp) , TEMP_STRING_SIZE-strlen(Temp), "v%d", Decoded->vC);
        if (Decoded->vA>1)
            sprintf_s(Temp+strlen(Temp) , TEMP_STRING_SIZE-strlen(Temp), " ,v%d", Decoded->vD);
        if (Decoded->vA>2)
            sprintf_s(Temp+strlen(Temp) , TEMP_STRING_SIZE-strlen(Temp), " ,v%d", Decoded->vE);
        if (Decoded->vA>3)
            sprintf_s(Temp+strlen(Temp) , TEMP_STRING_SIZE-strlen(Temp), " ,v%d", Decoded->vF);
        if (Decoded->vA==5)
            sprintf_s(Temp+strlen(Temp) , TEMP_STRING_SIZE-strlen(Temp), " ,v%d", Decoded->vG);

        sprintf_s(Temp+strlen(Temp) , TEMP_STRING_SIZE-strlen(Temp), "}, %s@%d",
            Opcode[0] == OP_FILLED_NEW_ARRAY? "class": "method",
            Decoded->vB);
        break;

    case OP_FORMAT_35ms:       // [opt] invoke-virtual+super
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_35fs:       // [opt] invoke-interface
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3rc:        // op {vCCCC .. v(CCCC+AA-1)}, meth@BBBB
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3rms:       // [opt] invoke-virtual+super/range
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3rfs:       // [opt] invoke-interface/range
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_3inline:    // [opt] inline invoke
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s", Decoded->Opcode);
        break;
    case OP_FORMAT_51l:        // op vAA, #+BBBBBBBBBBBBBBBB
        Decoded->vA = Opcode[1];
        Decoded->vB_wide = *(INT64*)&Opcode[2];
        Decoded->BytesSize += 9;
        sprintf_s(Temp, TEMP_STRING_SIZE, "%s v%d, #%s%I64d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT64)Decoded->vB_wide>=0?"+":"",
            Decoded->vB_wide);
        break;
    }

    Decoded->Decoded = new UCHAR[strlen(Temp)+1];
    strncpy_s((CHAR*)Decoded->Decoded, strlen(Temp)+1, Temp, TEMP_STRING_SIZE);
    free(Temp);
    return Decoded;
}



void cDexFile::DumpMethodParameters(
    UINT MethodIndex, 
    CLASS_METHOD* Method
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
    CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef
    )
{
    if ((*CodeAreaDef).TriesSize > 0)
    {
        (*CodeArea).Tries = new CLASS_CODE_TRY[(*CodeAreaDef).TriesSize];
        ZERO((*CodeArea).Tries, sizeof(CLASS_CODE_TRY) * (*CodeAreaDef).TriesSize);
                        
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
                &(*CodeArea).CatchHandlers);
    }
    else 
        (*CodeArea).Tries = NULL;
}

void cDexFile::DumpMethodCatchHandlers(
    CLASS_CODE* CodeArea, 
    UCHAR** Buffer
    )
{
    (*CodeArea).CatchHandlersSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

    if ((*CodeArea).CatchHandlersSize)
    {
        (*CodeArea).CatchHandlers = new CLASS_CODE_CATCH_HANDLER[(*CodeArea).CatchHandlersSize];

        ZERO((*CodeArea).CatchHandlers, sizeof(CLASS_CODE_CATCH_HANDLER) * (*CodeArea).CatchHandlersSize);

        for (UINT k=0; k<(*CodeArea).CatchHandlersSize; k++)
        {
            (*CodeArea).CatchHandlers[k].TypeHandlersSize = ReadSignedLeb128((CONST UCHAR**)Buffer);

            if ((*CodeArea).CatchHandlers[k].TypeHandlersSize)
            {
                (*CodeArea).CatchHandlers[k].TypeHandlers = new CLASS_CODE_CATCH_TYPE_PAIR[abs((*CodeArea).CatchHandlers[k].TypeHandlersSize)];
                ZERO((*CodeArea).CatchHandlers[k].TypeHandlers, 
                    sizeof(CLASS_CODE_CATCH_TYPE_PAIR) * abs((*CodeArea).CatchHandlers[k].TypeHandlersSize));

                for (UINT l=0; l<(UINT)abs((*CodeArea).CatchHandlers[k].TypeHandlersSize); l++)
                {
                    (*CodeArea).CatchHandlers[k].TypeHandlers[l].TypeIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
                    (*CodeArea).CatchHandlers[k].TypeHandlers[l].Address = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

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
                (*CodeArea).CatchHandlers[k].CatchAllAddress = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

            (*CodeArea).CatchHandlers[k].TypeHandlersSize = abs((*CodeArea).CatchHandlers[k].TypeHandlersSize);
        }
    }
}

void cDexFile::DumpMethodTryItemsInfo(
    CLASS_CODE_TRY* TryItem, 
    DEX_TRY_ITEM* TryItemInfo,
    CLASS_CODE_CATCH_HANDLER** CatchHandlers
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

        switch ((UCHAR)i) 
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

        OpcodesFormat[(UCHAR)i] = Format;
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

INT cDexFile::ReadSignedLeb128(
    CONST UCHAR** pStream
    ) 
{
 CONST UCHAR* ptr = *pStream;
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
    CONST UCHAR** pStream
    ) 
{
    CONST UCHAR* ptr = *pStream;
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
