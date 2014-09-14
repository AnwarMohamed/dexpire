#include "cDexFile.h"
#define NO_INDEX 0xffffffff

cDexFile::cDexFile(CHAR* Filename): cFile(Filename)
{
	isReady = BaseAddress && FileLength >= sizeof(DEX_HEADER) && DumpDex();
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
	DexClassDefs = (DEX_CLASS_DEF*)(BaseAddress + DexHeader->ClassDefsOff);

	for (UINT i=0; i<nClasses; i++)
	{
		DexClasses[i].Descriptor = StringItems[DexTypeIds[DexClassDefs[i].ClassIdx].StringIndex].Data;
		DexClasses[i].AccessFlags = DexClassDefs[i].AccessFlags;
		DexClasses[i].SuperClass = StringItems[DexTypeIds[DexClassDefs[i].SuperclassIdx].StringIndex].Data;

		if (DexClassDefs[i].SourceFileIdx != NO_INDEX)
			DexClasses[i].SourceFile = StringItems[DexClassDefs[i].SourceFileIdx].Data;
		else
			DexClasses[i].SourceFile = (UCHAR*)"No Information Found";

		if (DexClassDefs[i].ClassDataOff != NULL)
		{
			DexClasses[i].ClassData = new DEX_CLASS_STRUCTURE::CLASS_DATA;
			DexClassData = (DEX_CLASS_DATA*)(BaseAddress + DexClassDefs[i].ClassDataOff);

			BufPtr = (UCHAR*)DexClassData;

			DexClasses[i].ClassData->StaticFieldsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			DexClasses[i].ClassData->InstanceFieldsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			DexClasses[i].ClassData->DirectMethodsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);	
			DexClasses[i].ClassData->VirtualMethodsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			DexClasses[i].ClassData->InterfacesSize = DexClassDefs[i].InterfacesOff? *(UINT*)(BaseAddress + DexClassDefs[i].InterfacesOff): 0;

			DexClasses[i].ClassData->StaticFields = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD[DexClasses[i].ClassData->StaticFieldsSize];
			DexClasses[i].ClassData->InstanceFields = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD[DexClasses[i].ClassData->InstanceFieldsSize];
			DexClasses[i].ClassData->DirectMethods = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD[DexClasses[i].ClassData->DirectMethodsSize];
			DexClasses[i].ClassData->VirtualMethods = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD[DexClasses[i].ClassData->VirtualMethodsSize];
			
			DexClasses[i].ClassData->Interfaces = 
				new UCHAR*[DexClasses[i].ClassData->InterfacesSize];

			UINT CurIndex = 0;

			for (UINT j=0; j<DexClasses[i].ClassData->InterfacesSize; j++)
			{
				DexClasses[i].ClassData->Interfaces[j] = 
					StringItems[DexTypeIds[ ((USHORT*)(BaseAddress+DexClassDefs[i].InterfacesOff+sizeof(UINT)))[j] ].StringIndex].Data;
			}

			for (UINT j=0; j<DexClasses[i].ClassData->StaticFieldsSize; j++)
			{	
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->StaticFields[j].Type = StringItems[DexTypeIds[ DexFieldIds[CurIndex].TypeIdex ].StringIndex].Data;
				DexClasses[i].ClassData->StaticFields[j].Name = StringItems[DexFieldIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->StaticFields[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			CurIndex = 0;
			for (UINT j=0; j<DexClasses[i].ClassData->InstanceFieldsSize; j++)
			{
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->InstanceFields[j].Type = StringItems[DexTypeIds[DexFieldIds[CurIndex].TypeIdex].StringIndex].Data;
				DexClasses[i].ClassData->InstanceFields[j].Name = StringItems[DexFieldIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->InstanceFields[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			CurIndex = 0;
			for (UINT j=0; j<DexClasses[i].ClassData->DirectMethodsSize; j++)
			{
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->DirectMethods[j].ProtoType = 
					StringItems[DexTypeIds[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].ReturnTypeIdx].StringIndex].Data;

				//DexClasses[i].ClassData->DirectMethods[j].Type = StringItems[DexTypeIds[DexMethodIds[CurIndex].ClassIndex].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].Name = StringItems[DexMethodIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);

				UINT code_offset = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				if (code_offset == NULL)
					DexClasses[i].ClassData->DirectMethods[j].CodeArea = NULL;
				else
				{
					DexClasses[i].ClassData->VirtualMethods[j].CodeArea = new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE;
					DexCode = (DEX_CODE*)(BaseAddress + code_offset);
				}

			}

			CurIndex = 0;
			for (UINT j=0; j<DexClasses[i].ClassData->VirtualMethodsSize; j++)
			{
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->VirtualMethods[j].ProtoType = 
					StringItems[DexTypeIds[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].ReturnTypeIdx].StringIndex].Data;

				//DexClasses[i].ClassData->VirtualMethods[j].Type = 
				//	StringItems[DexTypeIds[DexMethodIds[CurIndex].ClassIndex].StringIndex].Data;

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

CHAR* cDexFile::GetAccessMask(UINT Type, UINT AccessFlags)
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

INT cDexFile::ReadUnsignedLeb128(const UCHAR** pStream) 
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
