/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#include "cDexFile.h"
#include <stdio.h>

cDexFile::cDexFile(CHAR* Filename): 
    cFile(Filename)
{
    isReady = (BaseAddress && FileLength >= sizeof(DEX_HEADER) && DumpDex());
}

cDexFile::cDexFile(CHAR* Buffer, DWORD Size):
    cFile(Buffer, Size)
{
    isReady = (BaseAddress && FileLength >= sizeof(DEX_HEADER) && DumpDex());
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
    DexClassDefs = (DEX_CLASS_DEF*)(BaseAddress + DexHeader->ClassDefsOff);

    /* Opcode Parsing Tables */
    CreateOpcodesWidthsTable();
    CreateOpcodesFlagsTable();
    CreateOpcodesFormatTable();

    CLASS_FIELD* Field;
    CLASS_METHOD* Method;

    DEX_CLASS_STRUCTURE* Class;
    for (UINT i=0; i<nClassDefinitions; i++)
    {
        Class = new DEX_CLASS_STRUCTURE;
        DumpClassInfo(i, Class);

        if (DexClassDefs[i].ClassDataOff != NULL)
        {
            Class->ClassData = new CLASS_DATA();

            DexClassData = (DEX_CLASS_DATA*)(BaseAddress + DexClassDefs[i].ClassDataOff);

            Ptr = (UCHAR*)DexClassData;
            DumpClassDataInfo(i, Class, &Ptr);

            /* Parsing Static Fields */
            UINT CurIndex = 0;
            for (UINT j=0; j<Class->ClassData->StaticFieldsSize; j++)
            {
                Field = new CLASS_FIELD;
                ZERO(Field, sizeof(CLASS_FIELD));
                Field->Parent = Class;
                DumpFieldByIndex(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    Field, &Ptr);
                Class->ClassData->StaticFields.push_back(Field);
            }

            /* Parsing Instance Fields */
            CurIndex = 0;
            for (UINT j=0; j<Class->ClassData->InstanceFieldsSize; j++)
            {
                Field = new CLASS_FIELD;
                ZERO(Field, sizeof(CLASS_FIELD));
                Field->Parent = Class;
                DumpFieldByIndex(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    Field, &Ptr);
                Class->ClassData->InstanceFields.push_back(Field);
            }

            /* Parsing Static Fields Vales */
            DumpFieldsValues(DexClassDefs[i].StaticValuesOff, Class->ClassData);

            /* Parsing Direct Methods */
            CurIndex = 0;
            for (UINT j=0; j<Class->ClassData->DirectMethodsSize; j++)
            {
                Method = new CLASS_METHOD;
                ZERO(Method, sizeof(CLASS_METHOD));
                Method->Parent = Class;
                DumpMethodById(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    Method, &Ptr);
                Class->ClassData->DirectMethods.push_back(Method);
            }

            /* Parsing Virtual Methods */
            CurIndex = 0;
            for (UINT j=0; j<Class->ClassData->VirtualMethodsSize; j++)
            {
                Method = new CLASS_METHOD;
                ZERO(Method, sizeof(CLASS_METHOD));
                Method->Parent = Class;
                DumpMethodById(
                    (CurIndex += ReadUnsignedLeb128((CONST UCHAR**)&Ptr)),
                    Method, &Ptr);
                Class->ClassData->VirtualMethods.push_back(Method);
            }

            /* Parsing Annotations */
            DumpAnnotations(Class, DexClassDefs[i].AnnotationsOff);
        }
        else
            Class->ClassData = NULL;

        DexClasses.push_back(Class);
    }

    return TRUE;
}

void cDexFile::DumpClassInfo(
    UINT ClassIndex, 
    DEX_CLASS_STRUCTURE* Class
    )
{
    Class->Ref = &DexClassDefs[ClassIndex];
    Class->Descriptor = StringItems[DexTypeIds[DexClassDefs[ClassIndex].ClassIdx].StringIndex].Data;
    Class->AccessFlags = DexClassDefs[ClassIndex].AccessFlags;
    Class->SuperClass = StringItems[DexTypeIds[DexClassDefs[ClassIndex].SuperclassIdx].StringIndex].Data;

    /* Allocating Interfaces */
    if (Class->Ref->InterfacesOff)
        for (UINT j=0; j<*(UINT*)(BaseAddress+DexClassDefs[ClassIndex].InterfacesOff); j++)
            DumpInterfaceByIndex(
                ClassIndex,
                j,
                Class->Interfaces);


    if (DexClassDefs[ClassIndex].SourceFileIdx != NO_INDEX)
        Class->SourceFile = StringItems[DexClassDefs[ClassIndex].SourceFileIdx].Data;
    else
    {
        CHAR* SourceFile = strrchr((CHAR*)Class->Descriptor, '/') + 1;
        UINT SourceFileLen = strlen(SourceFile)-1;

        (*Class).SourceFile = new UCHAR[SourceFileLen + 6];
        memcpy(Class->SourceFile, SourceFile, SourceFileLen);
        memcpy(Class->SourceFile + SourceFileLen, ".java", 6);
    }
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
    vector<string> &Interfaces
    )
{
    Interfaces.push_back((CHAR*)StringItems[DexTypeIds[ 
        ((USHORT*)(BaseAddress+DexClassDefs[ClassIndex].InterfacesOff + sizeof(UINT)))[InterfaceIndex] 
    ].StringIndex].Data);
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
    CHAR* Temp = new CHAR[MAX_STRING_BUFFER_SIZE];

    for (UINT i=0; i<ValuesSize; i++)
    {
        ValueType = *Ptr++;
        ValueSize = (ValueType >> 5) +1;

        ZERO(Temp,MAX_STRING_BUFFER_SIZE);

        switch(ValueType & 0x1F)
        {    
        case VALUE_BYTE:
            _itoa_s(*(CHAR*)Ptr, Temp, MAX_STRING_BUFFER_SIZE, 10);
            Ptr += 1;
            break;

        case VALUE_SHORT:
            _itoa_s((*(SHORT*)Ptr) << ((2-ValueSize)*8) >> ((2-ValueSize)*8), Temp, MAX_STRING_BUFFER_SIZE, 10);
            Ptr += ValueSize;
            break;

        case VALUE_CHAR:
            _itoa_s((*(UCHAR*)Ptr) << ((2-ValueSize)*8) >> ((2-ValueSize)*8) , Temp, MAX_STRING_BUFFER_SIZE, 10);
            Ptr += ValueSize;
            break;

        case VALUE_INT:
            _itoa_s((*(INT*)Ptr) << ((4-ValueSize)*8) >> ((4-ValueSize)*8), Temp, MAX_STRING_BUFFER_SIZE, 10);
            Ptr += ValueSize;
            break;

        case VALUE_LONG:
            _ltoa_s((*(LONG*)Ptr) << ((8-ValueSize)*8) >> ((8-ValueSize)*8), Temp, MAX_STRING_BUFFER_SIZE, 10);
            Ptr += ValueSize;
            break;

        case VALUE_FLOAT:
        case VALUE_DOUBLE:
            Ptr += ValueSize;
            break;

        case VALUE_STRING:
            sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "\"%s\"", StringItems[(*(UINT*)Ptr) << ((4-ValueSize)*8) >> ((4-ValueSize)*8)].Data);
            Ptr  += ValueSize;
            break;

        case VALUE_TYPE:
        case VALUE_FIELD:
        case VALUE_METHOD:
        case VALUE_ENUM:
            Ptr += ValueSize;
            break;

        case VALUE_ARRAY:

            printf("test-array");
            break;

        case VALUE_ANNOTATION:
            

            printf("test-anno");
            break;

        case VALUE_NULL:
            sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "null");
            break;

        case VALUE_BOOLEAN:
            sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", ValueSize+1?"true":"false");
            break;

        case VALUE_SENTINEL:
            break;
        }

        ClassData->StaticFields[i]->Value = new UCHAR[strlen(Temp)+1];
        memcpy(ClassData->StaticFields[i]->Value, Temp, strlen(Temp)+1);
    }

    delete Temp;
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

        CLASS_ANNOTATION* Annotation;
        for (UINT i=0; i<ClassAnnotations->Size; i++)
        {
            ClassAnnotationItem = (DEX_ANNOTATION_ITEM*)(BaseAddress + ClassAnnotations->Entries[i].AnnotationOff);
            Ptr = (UCHAR*)ClassAnnotationItem->Encoded;

            Annotation = new CLASS_ANNOTATION;
            ZERO(Annotation, sizeof(CLASS_ANNOTATION));

            Annotation->Type = StringItems[DexTypeIds[ReadUnsignedLeb128((CONST UCHAR**)&Ptr)].StringIndex].Data;
            Annotation->Visibility = ClassAnnotationItem->Visibility;
            Annotation->ElementsSize = ReadUnsignedLeb128((CONST UCHAR**)&Ptr);

            if (Annotation->ElementsSize)
            {
                Annotation->Elements = 
                    new CLASS_ANNOTATION_ELEMENT[Annotation->ElementsSize];
                ZERO(Annotation->Elements, Annotation->ElementsSize * sizeof(CLASS_ANNOTATION_ELEMENT));
            }

            for (UINT j=0; j<Annotation->ElementsSize; j++)
            {
                Annotation->Elements[j].Name = StringItems[ReadUnsignedLeb128((CONST UCHAR**)&Ptr)].Data;
                DumpAnnotationElementValue(&Annotation->Elements[j], &Ptr);
            }

            DexClass->ClassData->Annotations.push_back(Annotation);
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
    Method->ProtoType = StringItems[DexTypeIds[DexProtoIds[DexMethodIds[
        MethodIndex
    ].PrototypeIndex].ReturnTypeIdx].StringIndex].Data;

    /* Parsing Parameters */
    DumpMethodParameters(
        MethodIndex, 
        Method);
                
    Method->Name = StringItems[DexMethodIds[MethodIndex].StringIndex].Data;
    Method->AccessFlags = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

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
        Method->CodeArea = NULL;
    else
    {
        Method->CodeArea = new CLASS_CODE();
        //ZERO(Method->CodeArea, sizeof(CLASS_CODE));

        DumpMethodCodeInfo(
            Method->CodeArea,
            CodeAreaDef);
 
        /* Debug Info */
        if (CodeAreaDef->DebugInfoOff)
        {
            TempBuffer = (UCHAR*)(BaseAddress + CodeAreaDef->DebugInfoOff);
            DumpMethodDebugInfo(
                Method,
                &(Method->CodeArea->DebugInfo),
                &TempBuffer);
        }
                  
        /* Tries Parsing */
        DumpMethodTryItems(
            Method->CodeArea, 
            CodeAreaDef);

        /* Instructions Parsing */
        DumpMethodInstructions(
            Method->CodeArea,
            CodeAreaDef);
    }
}

void cDexFile::DumpMethodCodeInfo(
    CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef    
    )
{
    CodeArea->InsSize = CodeAreaDef->InsSize;
    CodeArea->RegistersSize = CodeAreaDef->RegistersSize;
    CodeArea->OutsSize = CodeAreaDef->OutsSize;
    CodeArea->TriesSize = CodeAreaDef->TriesSize;
    CodeArea->InstBufferSize = CodeAreaDef->InstructionsSize;
    CodeArea->Offset = 0;

    ZERO(&CodeArea->DebugInfo, sizeof(CLASS_CODE_DEBUG_INFO));
}

void cDexFile::DumpMethodLocals(
    CLASS_METHOD* Method, 
    UCHAR** Buffer)
{
    USHORT LocalsIndex = Method->CodeArea->RegistersSize - Method->CodeArea->InsSize;
    UINT LocalsSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

    CLASS_CODE_LOCAL* Local;

    if (!(Method->AccessFlags & ACC_STATIC))
    {
        Local = new CLASS_CODE_LOCAL;
        ZERO(Local, sizeof(CLASS_CODE_LOCAL));

        Local->Name = "this";
        Local->Type = cDexString::GetTypeDescription((CHAR*)Method->Parent->Descriptor);    
        Local->EndAddress = Method->CodeArea->InstBufferSize;
        
        Method->CodeArea->Locals[LocalsIndex++].push_back(Local);
    }
    
    UINT LocalNameIndex;
    for (UINT i=0; i<LocalsSize; i++)
    {
        LocalNameIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer) - 1;
        
        Local = new CLASS_CODE_LOCAL;
        ZERO(Local, sizeof(CLASS_CODE_LOCAL));

        if(LocalNameIndex != NO_INDEX && StringItems[LocalNameIndex].Data)
            Local->Name = (CHAR*)StringItems[LocalNameIndex].Data;  

        Local->Type = cDexString::GetTypeDescription((CHAR*)Method->Type[i].c_str());
        Local->EndAddress = Method->CodeArea->InstBufferSize;

        Method->CodeArea->Locals[LocalsIndex++].push_back(Local); 
    }
}

void cDexFile::DumpMethodDebugInfo(
    CLASS_METHOD* Method,
    CLASS_CODE_DEBUG_INFO* DebugInfo,
    UCHAR** Buffer
    )
{
    UINT line = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
    UINT address = 0, new_opcode;

    DumpMethodLocals(Method, Buffer);

    UINT RegisterNumber;
    UINT NameIndex;
    UINT TypeIndex;
    UINT SigIndex;

    CLASS_CODE_LOCAL* Local;

    UCHAR Opcode;
    while(TRUE)
    {
        Opcode = *(*Buffer)++;
        switch (Opcode) 
        {
        case DBG_END_SEQUENCE:
            for(map<UINT, vector<CLASS_CODE_LOCAL*>>::iterator iterator = Method->CodeArea->Locals.begin(); 
                iterator != Method->CodeArea->Locals.end(); iterator++) 
                for (UINT i=0; i<iterator->second.size(); i++)
                    if (!iterator->second[i]->EndAddress)
                        iterator->second[i]->EndAddress = Method->CodeArea->InstBufferSize;
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

            Local = new CLASS_CODE_LOCAL;
            ZERO(Local, sizeof(CLASS_CODE_LOCAL));

            Method->CodeArea->Locals[RegisterNumber].push_back(Local);

            NameIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1;
            if (NameIndex != NO_INDEX && StringItems[NameIndex].Data)
                Local->Name = (CHAR*)StringItems[NameIndex].Data;

            TypeIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1;
            if (TypeIndex != NO_INDEX)
                Local->Type = cDexString::GetTypeDescription((CHAR*)StringItems[DexTypeIds[TypeIndex].StringIndex].Data);

            if (Opcode == DBG_START_LOCAL_EXTENDED)
            {
                SigIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1;
                if (SigIndex != NO_INDEX)
                    Local->Signature = (CHAR*)StringItems[SigIndex].Data;
            }
            
            Local->StartAddress = address;
            break;

        case DBG_END_LOCAL:
            InsertDebugPosition(Method->CodeArea, line, address);
            
            RegisterNumber = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

            for (UINT i=0; i<Method->CodeArea->Locals[RegisterNumber].size(); i++)
                if (!Method->CodeArea->Locals[RegisterNumber][i]->EndAddress)
                {
                    Method->CodeArea->Locals[RegisterNumber][i]->EndAddress = address;
                    break;
                }
            break;

        case DBG_RESTART_LOCAL:
            RegisterNumber = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
            
            Local = new CLASS_CODE_LOCAL;
            memcpy(
                Method->CodeArea->Locals[RegisterNumber][Method->CodeArea->Locals[RegisterNumber].size()-1],
                Local, sizeof(CLASS_CODE_LOCAL));

            Local->StartAddress = address;
            Method->CodeArea->Locals[RegisterNumber].push_back(Local);
            break;

        case DBG_SET_PROLOGUE_END:
        case DBG_SET_EPILOGUE_BEGIN:
            break;

        case DBG_SET_FILE:
            Method->Parent->SourceFile = StringItems[ReadUnsignedLeb128((CONST UCHAR**)Buffer)-1].Data;
            break;
        default:
            new_opcode = Opcode - DBG_FIRST_SPECIAL; 
            line += DBG_LINE_BASE + (new_opcode % DBG_LINE_RANGE);
            address += new_opcode / DBG_LINE_RANGE;
            InsertDebugPosition(Method->CodeArea, line, address);
        };
    }
    
    for(map<UINT, vector<CLASS_CODE_LOCAL*>>::iterator iterator = Method->CodeArea->Locals.begin(); 
    iterator != Method->CodeArea->Locals.end(); iterator++) 
    if (iterator->second.size() == 1)
        iterator->second[0]->StartAddress = 0;
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
    if (CodeAreaDef->InstructionsSize > 0)
    {
        CodeArea->Offset = (DWORD)(CodeAreaDef) - BaseAddress;

        CHAR Opcode;
        CLASS_CODE_INSTRUCTION* Instruction;
        for (UINT i=0; i<CodeAreaDef->InstructionsSize;)
        {
            Opcode = LOW_BYTE(CodeAreaDef->Instructions[i]);

            Instruction = DecodeOpcode(&(*CodeAreaDef).Instructions[i]);
            Instruction->Offset = /*(DWORD)(CodeAreaDef->Instructions) - BaseAddress +*/ i;
            
            CodeArea->Instructions.push_back(Instruction);
            i+= Instruction->BufferSize; 
        }
    }
}

CLASS_CODE_INSTRUCTION* cDexFile::DecodeOpcode(
    USHORT* Opcode
    )
{
    CLASS_CODE_INSTRUCTION* Decoded = new CLASS_CODE_INSTRUCTION;
    ZERO(Decoded, sizeof(CLASS_CODE_INSTRUCTION));

    Decoded->OpcodeSig = LOW_BYTE(*Opcode);
    Decoded->Opcode = (UCHAR*)OpcodesStrings[LOW_BYTE(*Opcode)];
    Decoded->Format = (UCHAR*)OpcodesFormatStrings[OpcodesFormat[LOW_BYTE(*Opcode)]];
    Decoded->Buffer = (USHORT*)Opcode;
    Decoded->BufferSize = 1;

    CHAR* Temp = new CHAR[MAX_STRING_BUFFER_SIZE];

    switch(OpcodesFormat[LOW_BYTE(*Opcode)])
    {
    case OP_FORMAT_UNKNOWN:
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "<unknown>");
        break;

    case OP_FORMAT_10x:        // op
        if (LOW_BYTE(*Opcode) == OP_NOP) 
            switch(LOW_BYTE(*Opcode) | (HIGH_BYTE(*Opcode) << 8))
            {
            case kPackedSwitchSignature:
                sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "packed-switch-data");
                Decoded->BufferSize = (Opcode[1] * 2) + 4;
                break;

            case kSparseSwitchSignature:
                sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "sparse-switch-data");
                Decoded->BufferSize = (Opcode[1] * 4) + 2;
                break;

            case kArrayDataSignature:
                sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "array-data");
                Decoded->BufferSize = (Opcode[1] * (*(UINT*)&Opcode[2]) + 1) / 2 + 4;
                break;

            default:
                sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "nop");
            }
        else
        {
            sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        }
        break;

    case OP_FORMAT_12x:        // op vA, vB
        Decoded->vA = HIGH_BYTE(*Opcode) & 0x0F;
        Decoded->vB = HIGH_BYTE(*Opcode) >> 4;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_11n:        // op vA, #+B
        Decoded->vA = HIGH_BYTE(*Opcode) & 0x0F;
        Decoded->vB = (CHAR)(HIGH_BYTE(*Opcode) >> 4);
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode, 
            Decoded->vA,
            (CHAR)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_11x:        // op vAA
        Decoded->vA = HIGH_BYTE(*Opcode);
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d", 
            Decoded->Opcode, 
            Decoded->vA);
        break;

    case OP_FORMAT_10t:        // op +AA
        Decoded->vA = (CHAR)HIGH_BYTE(*Opcode);
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s %s%d", 
            Decoded->Opcode, 
            (CHAR)Decoded->vA>=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_20bc:       // op AA, thing@BBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = Opcode[1];
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s %d, %s@%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            "thing", 
            Decoded->vB);
        break;

    case OP_FORMAT_20t:        // op +AAAA
        Decoded->vA = *(SHORT*)HALF_SHORT(Opcode);
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s %s%d", 
            Decoded->Opcode, 
            (SHORT)Decoded->vA>=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_22x:        // op vAA, vBBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = Opcode[1];
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB);
        break;

    case OP_FORMAT_21t:        // op vAA, +BBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = (SHORT)Opcode[1];
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, %s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21s:        // op vAA, #+BBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = (SHORT)Opcode[1];
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21h:        // op vAA, #+BBBB00000[00000000]
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = (SHORT)Opcode[1];
        Decoded->BufferSize += 1;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            (SHORT)Decoded->vB>=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_21c:        // op vAA, thing@BBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = Opcode[1];
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, %s@%d", 
            Decoded->Opcode, 
            Decoded->vA,
            LOW_BYTE(*Opcode) == OP_CONST_STRING? "string" :
            LOW_BYTE(*Opcode) == OP_CHECK_CAST || LOW_BYTE(*Opcode) == OP_CONST_CLASS? "class" :
            LOW_BYTE(*Opcode) == OP_NEW_INSTANCE? "type" :
            "field",
            Decoded->vB);
        break;

    case OP_FORMAT_23x:        // op vAA, vBB, vCC
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = LOW_BYTE(Opcode[1]);
        Decoded->vC = HIGH_BYTE(Opcode[1]);
        Decoded->BufferSize += 1;
        
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d, v%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            Decoded->vC);
        break;

    case OP_FORMAT_22b:        // op vAA, vBB, #+CC
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = LOW_BYTE(Opcode[1]);
        Decoded->vC = (CHAR)HIGH_BYTE(Opcode[1]);
        Decoded->BufferSize += 1;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            (CHAR)Decoded->vC>=0?"+":"",
            Decoded->vC);
        break;

    case OP_FORMAT_22t:        // op vA, vB, +CCCC
        Decoded->vA = HIGH_BYTE(*Opcode) & 0x0F;
        Decoded->vB = HIGH_BYTE(*Opcode) >> 4;
        Decoded->vC = (SHORT)Opcode[1];
        Decoded->BufferSize += 1;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d, %s%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            (SHORT)Decoded->vC>=0?"+":"",
            Decoded->vC);
        break;

    case OP_FORMAT_22s:        // op vA, vB, #+CCCC
        Decoded->vA = HIGH_BYTE(*Opcode) & 0x0F;
        Decoded->vB = HIGH_BYTE(*Opcode) >> 4;
        Decoded->vC = (SHORT)Opcode[1];
        Decoded->BufferSize += 1;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d, #%s%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            (SHORT)Decoded->vC>=0?"+":"",
            Decoded->vC);
        break;

    case OP_FORMAT_22c:        // op vA, vB, thing@CCCC
        Decoded->vA = HIGH_BYTE(*Opcode) & 0x0F;
        Decoded->vB = HIGH_BYTE(*Opcode) >> 4;
        Decoded->vC = (SHORT)Opcode[1];
        Decoded->BufferSize += 1;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d, %s@%d", 
            Decoded->Opcode,
            Decoded->vA,
            Decoded->vB,
            LOW_BYTE(*Opcode) >= OP_IGET && LOW_BYTE(*Opcode) <= OP_IPUT_SHORT? "field" :
            "class",
            Decoded->vC);
        break;

    case OP_FORMAT_22cs:       // [opt] op vA, vB, field offset CCCC
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_32x:        // op vAAAA, vBBBB
        Decoded->vA = *(USHORT*)HALF_SHORT(Opcode);
        Decoded->vB = *(USHORT*)HALF_SHORT(&Opcode[1]);
        Decoded->BufferSize += 2;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, v%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_30t:        // op +AAAAAAAA
        Decoded->vA = *(INT*)HALF_SHORT(Opcode);
        Decoded->BufferSize += 2;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s %s%d", 
            Decoded->Opcode,
            (INT)Decoded->vA<=0?"+":"",
            Decoded->vA);
        break;

    case OP_FORMAT_31t:        // op vAA, +BBBBBBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = *(INT*)HALF_SHORT(&Opcode[1]);
        Decoded->BufferSize += 2;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, %s%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT)Decoded->vB<=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_31i:        // op vAA, #+BBBBBBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = *(INT*)HALF_SHORT(&Opcode[1]);
        Decoded->BufferSize += 2;

        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, #%s%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT)Decoded->vB<=0?"+":"",
            Decoded->vB);
        break;

    case OP_FORMAT_31c:        // op vAA, thing@BBBBBBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB = *(UINT*)HALF_SHORT(&Opcode[1]);
        Decoded->BufferSize += 5;
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, string@%d", 
            Decoded->Opcode, 
            Decoded->vA, 
            Decoded->vB);
        break;

    case OP_FORMAT_35c:        // op {vC, vD, vE, vF, vG}, thing@BBBB (B: count, A: vG)
        Decoded->vA = HIGH_BYTE(*Opcode) >> 4;
        Decoded->vB = Opcode[1];
        Decoded->vC = Decoded->vA>0? LOW_BYTE(Opcode[2]) & 0x0F :0;
        Decoded->vArg[0] = Decoded->vA>1? LOW_BYTE(Opcode[2]) >> 4 :0;
        Decoded->vArg[1] = Decoded->vA>2? HIGH_BYTE(Opcode[2]) & 0x0F :0;
        Decoded->vArg[2] = Decoded->vA>3? HIGH_BYTE(Opcode[2]) >> 4 :0;
        Decoded->vArg[3] = Decoded->vA==5? LOW_BYTE(Opcode[3]) & 0x0F :0;
        
        Decoded->BufferSize += 2;
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s {", Decoded->Opcode);
        
        if (Decoded->vA)
            sprintf_s(Temp+strlen(Temp) , MAX_STRING_BUFFER_SIZE-strlen(Temp), "v%d", Decoded->vC);

        for (UINT i=1; i<Decoded->vA; i++)
            sprintf_s(Temp+strlen(Temp) , MAX_STRING_BUFFER_SIZE-strlen(Temp), " ,v%d", Decoded->vArg[i]);

        sprintf_s(Temp+strlen(Temp) , MAX_STRING_BUFFER_SIZE-strlen(Temp), "}, %s@%d",
            LOW_BYTE(*Opcode) == OP_FILLED_NEW_ARRAY? "class": "method",
            Decoded->vB);
        break;

    case OP_FORMAT_35ms:       // [opt] invoke-virtual+super
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_35fs:       // [opt] invoke-interface
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_3rc:        // op {vCCCC .. v(CCCC+AA-1)}, meth@BBBB
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_3rms:       // [opt] invoke-virtual+super/range
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_3rfs:       // [opt] invoke-interface/range
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_3inline:    // [opt] inline invoke
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s", Decoded->Opcode);
        break;

    case OP_FORMAT_51l:        // op vAA, #+BBBBBBBBBBBBBBBB
        Decoded->vA = HIGH_BYTE(*Opcode);
        Decoded->vB_wide = *(INT64*)HALF_SHORT(&Opcode[1]);
        Decoded->BufferSize += 4;
        sprintf_s(Temp, MAX_STRING_BUFFER_SIZE, "%s v%d, #%s%I64d", 
            Decoded->Opcode, 
            Decoded->vA, 
            (INT64)Decoded->vB_wide>=0?"+":"",
            Decoded->vB_wide);
        break;
    }

    Decoded->Decoded = Temp;
    delete Temp;
    return Decoded;
}

void cDexFile::DumpMethodParameters(
    UINT MethodIndex, 
    CLASS_METHOD* Method
    )
{
    if (DexProtoIds[DexMethodIds[MethodIndex].PrototypeIndex].ParametersOff)
    {
        DEX_TYPE_LIST* ParametersList = (DEX_TYPE_LIST*)(BaseAddress + DexProtoIds[DexMethodIds[
            MethodIndex
        ].PrototypeIndex].ParametersOff);
                
        for (UINT k=0; k<ParametersList->Size; k++) 
            Method->Type.push_back((CHAR*)StringItems[DexTypeIds[ParametersList->List[k].TypeIdx].StringIndex].Data);
    }
}

void cDexFile::DumpMethodTryItems(
    CLASS_CODE* CodeArea,
    DEX_CODE* CodeAreaDef
    )
{
    if (CodeAreaDef->TriesSize > 0)
    {
        USHORT* InstructionsEnd = &(CodeAreaDef->Instructions)[CodeAreaDef->InstructionsSize];
        if ((((UINT)InstructionsEnd) & 3) != 0) { InstructionsEnd++; }
        DEX_TRY_ITEM* TryItems = (DEX_TRY_ITEM*)InstructionsEnd;

        /* Parsing Catch Handlers */
        TempBuffer = (UCHAR*)&(TryItems[(*CodeAreaDef).TriesSize]);
        DumpMethodCatchHandlers(
            CodeArea, 
            &TempBuffer);

        for (UINT k=0; k<CodeAreaDef->TriesSize; k++)
            DumpMethodTryItemsInfo(
                CodeArea->Tries,
                &TryItems[k],
                CodeArea->CatchHandlers);
    }
}

void cDexFile::DumpMethodCatchHandlers(
    CLASS_CODE* CodeArea, 
    UCHAR** Buffer
    )
{
    CodeArea->CatchHandlersSize = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

    if (CodeArea->CatchHandlersSize)
    {
        CLASS_CODE_CATCH_HANDLER* CatchHandler;
        for (UINT k=0; k<(*CodeArea).CatchHandlersSize; k++)
        {
            CatchHandler = new CLASS_CODE_CATCH_HANDLER;
            ZERO(CatchHandler, sizeof(CLASS_CODE_CATCH_HANDLER));

            CatchHandler->TypeHandlersSize = ReadSignedLeb128((CONST UCHAR**)Buffer);

            if (CatchHandler->TypeHandlersSize)
            {
                CatchHandler->TypeHandlers = new CLASS_CODE_CATCH_TYPE_PAIR[abs(CatchHandler->TypeHandlersSize)];
                ZERO(CatchHandler->TypeHandlers, sizeof(CLASS_CODE_CATCH_TYPE_PAIR) * abs(CatchHandler->TypeHandlersSize));

                for (UINT l=0; l<(UINT)abs(CatchHandler->TypeHandlersSize); l++)
                {
                    CatchHandler->TypeHandlers[l].TypeIndex = ReadUnsignedLeb128((CONST UCHAR**)Buffer);
                    CatchHandler->TypeHandlers[l].Address = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

                    if (CatchHandler->TypeHandlers[l].TypeIndex == NO_INDEX)
                        CatchHandler->TypeHandlers[l].Type = (UCHAR*)"<any>";
                    else
                        CatchHandler->TypeHandlers[l].Type =
                            StringItems[DexTypeIds[ CatchHandler->TypeHandlers[l].TypeIndex ].StringIndex].Data;
                }
            }

            if (CatchHandler->TypeHandlersSize > 0)
                CatchHandler->CatchAllAddress = NULL;
            else
                CatchHandler->CatchAllAddress = ReadUnsignedLeb128((CONST UCHAR**)Buffer);

            CatchHandler->TypeHandlersSize = abs(CatchHandler->TypeHandlersSize);
        
            CodeArea->CatchHandlers.push_back(CatchHandler);
        }
    }
}

void cDexFile::DumpMethodTryItemsInfo(
    vector<CLASS_CODE_TRY*> &TryItems, 
    DEX_TRY_ITEM* TryItemInfo,
    vector<CLASS_CODE_CATCH_HANDLER*> &CatchHandlers
    )
{
    CLASS_CODE_TRY* TryItem = new CLASS_CODE_TRY;
    ZERO(TryItem, sizeof(CLASS_CODE_TRY));

    TryItem->InstructionsStart = TryItemInfo->StartAddress;
    TryItem->InstructionsEnd = TryItemInfo->StartAddress + TryItemInfo->InstructionsSize;
    //TryItem->CatchHandler = CatchHandlers[TryItemInfo->HandlerOff-1];

    TryItems.push_back(TryItem);
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

cDexFile::~cDexFile()
{
    if (!isReady) 
        return;

    //free(StringItems);

    //delete[] OpcodesWidths;
    //delete[] OpcodesFlags;
    //delete[] OpcodesFormat;

    for (UINT i=0; i<DexClasses.size(); i++)
    {

    }

    //free(DexClasses);
}
