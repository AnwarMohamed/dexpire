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

#include "..\src\cDexFile.h"
#include "..\src\cDexDecompiler.h"
#include "..\src\cApkFile.h"
#include <stdio.h>


#define DECOMPILED_LINES

int main()
{
    /*
    cApkFile* apk  = new cApkFile("test.apk");
    if (apk->isReady)
    {
        for (unsigned int i=0; i<apk->FilesCount; i++)
            if (strcmp(apk->Files[i].Name, "classes.dex") == 0)
            {
                cDexFile* dexFile = new cDexFile(apk->Files[i].Buffer, apk->Files[i].Size);
                if (dexFile->isReady)
                {
                    cDexDecompiler* dexDecompiler = new cDexDecompiler(dexFile);

                }
                break;
            }
            else if (strcmp(apk->Files[i].Name + strlen(apk->Files[i].Name) - 4, ".xml") == 0)
            {
                cBinXMLFile* xml = new cBinXMLFile(apk->Files[i].Buffer, apk->Files[i].Size);
            }
    }
    */
    
    
    
    //cDexFile* dex = new cDexFile("classes.dex");
    cDexFile* dex = new cDexFile("C:\\Downloads\\finfisher\\qateam\\ak\\ANDR\\classes.dex");
    printf("Processing '%s'...\n", dex->Filename);

    if (dex->isReady)
    {
        printf("Opened '%s', DEX version '%s'\n\n\n", dex->Filename, dex->DexVersion);
        cDexDecompiler* decompiled = new cDexDecompiler(dex);
        /*
        for (UINT i=0; i<dex->nClasses; i++)
        {
            printf("package %s;\n\n", decompiled->Classes[i].Package);

            for (UINT j=0; j<decompiled->Classes[i].ImportsSize; j++)
            {
                printf("import %s;\n", decompiled->Classes[i].Imports[j]);
                if (j+1 == decompiled->Classes[i].ImportsSize)
                    printf("\n");
            }
            
            printf("%s class %s", 
                decompiled->Classes[i].AccessFlags,
                decompiled->Classes[i].Name);

            if (decompiled->Classes[i].ExtendsSize)
                printf(" extends ");

            for (UINT j=0; j<decompiled->Classes[i].ExtendsSize; j++)
            {
                if (j) printf(",");
                printf("%s ", cDexString::ExtractShortLType(decompiled->Classes[i].Extends[j]));
            }

            printf("{\n\n");
            

            for (UINT j=0; j<decompiled->Classes[i].FieldsSize; j++)
            {
                printf("    %s %s %s%s%s;\n",
                    decompiled->Classes[i].Fields[j]->AccessFlags,
                    decompiled->Classes[i].Fields[j]->ReturnType,
                    decompiled->Classes[i].Fields[j]->Name,
                    decompiled->Classes[i].Fields[j]->Static? (decompiled->Classes[i].Fields[j]->Value?" = ":""):"",
                    decompiled->Classes[i].Fields[j]->Static? (decompiled->Classes[i].Fields[j]->Value?decompiled->Classes[i].Fields[j]->Value:""):"");
                if (j+1 == decompiled->Classes[i].FieldsSize)
                    printf("\n");
            }
            
            
            for (UINT j=0; j<decompiled->Classes[i].MethodsSize; j++)
            {
                if (!decompiled->Classes[i].Methods[j]->Name ||
                    strcmp(decompiled->Classes[i].Methods[j]->Name, "<init>") == 0 ||
                    strcmp(decompiled->Classes[i].Methods[j]->Name, "<clinit>") == 0)
                    continue;

                printf("    %s %s %s(",
                    decompiled->Classes[i].Methods[j]->AccessFlags,
                    cDexString::ExtractShortLType(decompiled->Classes[i].Methods[j]->ReturnType),
                    decompiled->Classes[i].Methods[j]->Name);

                for (UINT k=0; k<decompiled->Classes[i].Methods[j]->ArgumentsSize; k++)
                {
                    if (k) printf(", ");
                    printf("%s %s",
                        cDexString::ExtractShortLType(decompiled->Classes[i].Methods[j]->Arguments[k]->Type),
                        cDexString::ExtractShortLType(decompiled->Classes[i].Methods[j]->Arguments[k]->Name));
                }
                printf(") {\n");


                for (UINT k=0; k<decompiled->Classes[i].Methods[j]->LinesSize; k++)
                {
#ifdef DECOMPILED_LINES
                    if (decompiled->Classes[i].Methods[j]->Lines[k]->Decompiled)
                    {
                        printf("        %s;\n", decompiled->Classes[i].Methods[j]->Lines[k]->Decompiled);
                    }
                    else
                    {
                        
                        for (UINT l=0; l<decompiled->Classes[i].Methods[j]->Lines[k]->InstructionsSize; l++)
                        {
                            if (k && !l) printf("\n");
                            printf("        %s\n", decompiled->Classes[i].Methods[j]->Lines[k]->Instructions[l]->Decoded);
                        }
                    }
#else
                    for (UINT l=0; l<decompiled->Classes[i].Methods[j]->Lines[k]->InstructionsSize; l++)
                    {
                        if (k && !l) printf("\n");
                        printf("        %s;\n", decompiled->Classes[i].Methods[j]->Lines[k]->Instructions[l]->Decompiled);
                    }
#endif
                }


                printf("    }\n\n");
            }
            

            printf("}\n\n");
 
        }
        */
        
        delete decompiled;
        

        //for (UINT i=0; i<301 /*dex->nClassDefinitions*///; i++)
        /*{
            printf("Class #%d header:\n", i);
            printf(	"class_idx           : %d\n"
                    "access_flags        : %d (0x%04x)\n"
                    "superclass_idx      : %d\n"
                    "interfaces_off      : %d (0x%06x)\n"
                    "source_file_idx     : %d\n"
                    "annotations_off     : %d (0x%x)\n"
                    "class_data_off      : %d (0x%x)\n"
                    "static_fields_size  : %d\n"
                    "instance_fields_size: %d\n"
                    "direct_methods_size : %d\n"
                    "virtual_methods_size: %d\n\n",
                    
                    dex->DexClassDefs[i].ClassIdx,
                    dex->DexClassDefs[i].AccessFlags, dex->DexClassDefs[i].AccessFlags,
                    dex->DexClassDefs[i].SuperclassIdx,
                    dex->DexClassDefs[i].InterfacesOff, dex->DexClassDefs[i].InterfacesOff,
                    dex->DexClassDefs[i].SourceFileIdx,
                    dex->DexClassDefs[i].AnnotationsOff, dex->DexClassDefs[i].AnnotationsOff,
                    dex->DexClassDefs[i].ClassDataOff, dex->DexClassDefs[i].ClassDataOff,
                    dex->DexClassDefs[i].ClassDataOff ? dex->DexClasses[i].ClassData->StaticFieldsSize: 0,
                    dex->DexClassDefs[i].ClassDataOff ? dex->DexClasses[i].ClassData->InstanceFieldsSize: 0,
                    dex->DexClassDefs[i].ClassDataOff ? dex->DexClasses[i].ClassData->DirectMethodsSize: 0,
                    dex->DexClassDefs[i].ClassDataOff ? dex->DexClasses[i].ClassData->VirtualMethodsSize: 0);

            printf(	"Class #%d            -\n"
                    "  Class descriptor  : '%s'\n"
                    "  Access flags      : 0x%04x (%s)\n"
                    "  Superclass        : '%s'\n",
                    
                    i,
                    dex->DexClasses[i].Descriptor,
                    dex->DexClasses[i].AccessFlags, cDexString::GetAccessMask(0, dex->DexClasses[i].AccessFlags),
                    dex->DexClasses[i].SuperClass);


            printf(	"  Interfaces        -\n");
			for (UINT j=0; j<(dex->DexClassDefs[i].ClassDataOff?dex->DexClasses[i].ClassData->InterfacesSize:0); j++)
			{
				printf(	"    #%d              : '%s'\n", 
						j, dex->DexClasses[i].ClassData->Interfaces[j]);
			}


            printf(	"  Static fields     -\n");
			for (UINT j=0; j<(dex->DexClassDefs[i].ClassDataOff?dex->DexClasses[i].ClassData->StaticFieldsSize:0); j++)
			{
				printf("    #%d              : (in %s)\n"
					   "      name          : '%s'\n"
					   "      type          : '%s'\n"
					   "      access        : 0x%04x (%s)\n",
					   
					   j, dex->DexClasses[i].Descriptor,
					   dex->DexClasses[i].ClassData->StaticFields[j].Name,
					   dex->DexClasses[i].ClassData->StaticFields[j].Type,
					   dex->DexClasses[i].ClassData->StaticFields[j].AccessFlags, 
					   cDexString::GetAccessMask(2, dex->DexClasses[i].ClassData->StaticFields[j].AccessFlags)
					   );
			}


            printf(	"  Instance fields   -\n");
			for (UINT j=0; j<(dex->DexClassDefs[i].ClassDataOff?dex->DexClasses[i].ClassData->InstanceFieldsSize:0); j++)
			{
				printf("    #%d              : (in %s)\n"
					   "      name          : '%s'\n"
					   "      type          : '%s'\n"
					   "      access        : 0x%04x (%s)\n",
					   
					   j, dex->DexClasses[i].Descriptor,
					   dex->DexClasses[i].ClassData->InstanceFields[j].Name,
					   dex->DexClasses[i].ClassData->InstanceFields[j].Type,
					   dex->DexClasses[i].ClassData->InstanceFields[j].AccessFlags, 
					   cDexString::GetAccessMask(2, dex->DexClasses[i].ClassData->InstanceFields[j].AccessFlags)
					   );
			}


            printf(	"  Direct methods    -\n");
            for (UINT j=0; j<(dex->DexClassDefs[i].ClassDataOff?dex->DexClasses[i].ClassData->DirectMethodsSize:0); j++)
            {
                printf ("    #%d              : (in %s)\n"
                        "      name          : '%s'\n"
                        "      type          : '(%s)%s'\n"
                        "      access        : 0x%04x (%s)\n",

                        j, dex->DexClasses[i].Descriptor,
                        dex->DexClasses[i].ClassData->DirectMethods[j].Name,
						dex->DexClasses[i].ClassData->DirectMethods[j].Type, dex->DexClasses[i].ClassData->DirectMethods[j].ProtoType,
						dex->DexClasses[i].ClassData->DirectMethods[j].AccessFlags,  
						cDexString::GetAccessMask(1, dex->DexClasses[i].ClassData->DirectMethods[j].AccessFlags));

                if (dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea)
                {
                    printf( "      code          -\n"
                            "      registers     : %d\n"
                            "      ins           : %d\n"
                            "      outs          : %d\n"
                            "      insns size    : %d 16-bit code units\n",

						    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->RegistersSize,
						    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->InsSize,
						    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->OutsSize,
						    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->InstBufferSize);


                    printf("%06x:                                        |[%06x] %s.%s:(%s)%s\n", 
                        dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex->DexClasses[i].Descriptor,
                        dex->DexClasses[i].ClassData->DirectMethods[j].Name,
                        dex->DexClasses[i].ClassData->DirectMethods[j].Type? dex->DexClasses[i].ClassData->DirectMethods[j].Type: (UCHAR*)"",
                        dex->DexClasses[i].ClassData->DirectMethods[j].ProtoType);

                    UINT line = 0;
                    for (UINT k=0; k<dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->InstructionsSize; k++)
                    {
                        printf("%06x: ", dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->Offset & 0x00FFFFFF);
                        for (UINT l=0; l<(UINT)(dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BufferSize>7?7:
                            dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BufferSize); l++)
                            printf("%04x ", SWAP_SHORT(dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->Buffer[l]));

                        for (UINT l=0; l<39 - (UINT)(dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BufferSize>7?7:
                            dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BufferSize)*5; l++)
                            printf(" ");

                        printf("|%04x: %s\n", line,
                            dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->Decoded);
                        line += dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BufferSize/2;
                    }

				    if (!dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize)
					    printf("      catches       : (none)\n");
				    else
				    {
					    printf("      catches       : %d\n", 
					    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize);

					    for (UINT k=0; k<dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize; k++)
					    {
						    printf("        0x%04x - 0x%04x\n", 
							    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].InstructionsStart,
							    dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].InstructionsEnd);

                            //for (UINT l=0; l<(UINT)dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlersSize; l++)
                            //{
                            //    printf("          %s -> 0x%04x\n", 
                            //        dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Type,
                            //        dex->DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Address);
                            //}

					    }
				    }
                }
                else
                    printf( "      code          : (none)\n");
            }


            
			printf(	"  Virtual methods   -\n");
            for (UINT j=0; j<(dex->DexClassDefs[i].ClassDataOff?dex->DexClasses[i].ClassData->VirtualMethodsSize:0); j++)
            {
                printf ("    #%d              : (in %s)\n"
                        "      name          : '%s'\n"
                        "      type          : '(%s)%s'\n"
                        "      access        : 0x%04x (%s)\n",

                        j, dex->DexClasses[i].Descriptor,
                        dex->DexClasses[i].ClassData->VirtualMethods[j].Name,
						dex->DexClasses[i].ClassData->VirtualMethods[j].Type, dex->DexClasses[i].ClassData->VirtualMethods[j].ProtoType,
						dex->DexClasses[i].ClassData->VirtualMethods[j].AccessFlags,  
						cDexString::GetAccessMask(1, dex->DexClasses[i].ClassData->VirtualMethods[j].AccessFlags));

                if (dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea)
                {
                    printf( "      code          -\n"
                            "      registers     : %d\n"
                            "      ins           : %d\n"
                            "      outs          : %d\n"
                            "      insns size    : %d 16-bit code units\n",
						    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->RegistersSize,
						    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InsSize,
						    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->OutsSize,
						    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InstBufferSize);
                
                    printf("%06x:                                        |[%06x] %s.%s:(%s)%s\n", 
                        dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex->DexClasses[i].Descriptor,
                        dex->DexClasses[i].ClassData->VirtualMethods[j].Name,
                        dex->DexClasses[i].ClassData->VirtualMethods[j].Type? dex->DexClasses[i].ClassData->VirtualMethods[j].Type: (UCHAR*)"",
                        dex->DexClasses[i].ClassData->VirtualMethods[j].ProtoType);

                    UINT line = 0;
                    for (UINT k=0; k<dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InstructionsSize; k++)
                    {
                        printf("%06x: ", dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->Offset & 0x00FFFFFF);
                        for (UINT l=0; 
                            l<(UINT)(dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BufferSize>7?7: 
                            dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BufferSize); l++)
                            printf("%04x ", SWAP_SHORT(dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->Buffer[l]));

                        for (UINT l=0; l<39 - (UINT)(dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BufferSize>7?7: 
                            dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BufferSize)*5; l++)
                            printf(" ");

                        printf("|%04x: %s\n", line,
                            dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->Decoded);
                        line += dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BufferSize/2;
                    }

				    if (!dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize)
					    printf("      catches       : (none)\n");
				    else
				    {
					    printf("      catches       : %d\n", 
					    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize);

					    for (UINT k=0; k<dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize; k++)
					    {
						    printf("        0x%04x - 0x%04x\n", 
							    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].InstructionsStart,
							    dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].InstructionsEnd);

                            //for (UINT l=0; l<(UINT)dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlersSize; l++)
                            //{
                            //    printf("          %s -> 0x%04x\n", 
                            //        dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Type,
                            //        dex->DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Address);
                            //}

					    }
				    }
                }
                else
                    printf( "      code          : (none)\n");

                }
			

			printf("  source_file_idx   : %d (%s)\n\n", 
                dex->DexClassDefs[i].SourceFileIdx,
                dex->DexClasses[i].SourceFile);
                
        }
        */
            
        system("pause");
    }
    else 
        printf("Unable to load your dex file\n");

    delete dex;
    
    return 0;
    
}