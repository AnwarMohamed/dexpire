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
#include <stdio.h>

int main()
{
    cDexFile dex("classes.dex");
    printf("Processing '%s'...\n", dex.Filename);

    if (dex.isReady)
    {
        printf("Opened '%s', DEX version '%s'\n\n\n", dex.Filename, dex.DexVersion);
        cDexDecompiler decompiled(&dex);

        for (UINT i=300; i<301/*dex.nClassDefinitions*/; i++)
        {
            printf("package %s;\n\n", decompiled.Classes[i].Package);

            for (UINT j=0; j<decompiled.Classes[i].ImportsSize; j++)
                printf("import %s;\n", decompiled.Classes[i].Imports[j]);
            printf("\n");

            printf("%s class %s", 
                decompiled.Classes[i].AccessFlags,
                decompiled.Classes[i].Name);

            if (decompiled.Classes[i].ExtendsSize)
                printf(" extends ");

            for (UINT j=0; j<decompiled.Classes[i].ExtendsSize; j++)
            {
                if (j) printf(",");
                printf("%s ", decompiled.Classes[i].Extends[j]);
            }

            printf("{\n\n");
            
            for (UINT j=0; j<decompiled.Classes[i].MethodsSize; j++)
            {
                if (decompiled.Classes[i].Methods[j]->Virtual)
                    printf("    @Override\n");

                printf("    %s %s %s(",
                    decompiled.Classes[i].Methods[j]->AccessFlags,
                    decompiled.Classes[i].Methods[j]->ReturnType,
                    decompiled.Classes[i].Methods[j]->Name);

                for (UINT k=0; k<decompiled.Classes[i].Methods[j]->ArgumentsSize; k++)
                {
                    if (k) printf(", ");
                    printf("%s arg%d",
                        decompiled.Classes[i].Methods[j]->Arguments[k]->Type,
                        k);
                }
                printf(") {\n");



                printf("    }\n\n");
            }

            printf("}\n\n");
        }

        //for (UINT i=1; i< 2/*dex.nClassDefinitions*/; i++)
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
                    
                    dex.DexClassDefs[i].ClassIdx,
                    dex.DexClassDefs[i].AccessFlags, dex.DexClassDefs[i].AccessFlags,
                    dex.DexClassDefs[i].SuperclassIdx,
                    dex.DexClassDefs[i].InterfacesOff, dex.DexClassDefs[i].InterfacesOff,
                    dex.DexClassDefs[i].SourceFileIdx,
                    dex.DexClassDefs[i].AnnotationsOff, dex.DexClassDefs[i].AnnotationsOff,
                    dex.DexClassDefs[i].ClassDataOff, dex.DexClassDefs[i].ClassDataOff,
                    dex.DexClassDefs[i].ClassDataOff ? dex.DexClasses[i].ClassData->StaticFieldsSize: 0,
                    dex.DexClassDefs[i].ClassDataOff ? dex.DexClasses[i].ClassData->InstanceFieldsSize: 0,
                    dex.DexClassDefs[i].ClassDataOff ? dex.DexClasses[i].ClassData->DirectMethodsSize: 0,
                    dex.DexClassDefs[i].ClassDataOff ? dex.DexClasses[i].ClassData->VirtualMethodsSize: 0);

            printf(	"Class #%d            -\n"
                    "  Class descriptor  : '%s'\n"
                    "  Access flags      : 0x%04x (%s)\n"
                    "  Superclass        : '%s'\n",
                    
                    i,
                    dex.DexClasses[i].Descriptor,
                    dex.DexClasses[i].AccessFlags, dex.GetAccessMask(0, dex.DexClasses[i].AccessFlags),
                    dex.DexClasses[i].SuperClass);


            printf(	"  Interfaces        -\n");
			for (UINT j=0; j<(dex.DexClassDefs[i].ClassDataOff?dex.DexClasses[i].ClassData->InterfacesSize:0); j++)
			{
				printf(	"    #%d              : '%s'\n", 
						j, dex.DexClasses[i].ClassData->Interfaces[j]);
			}


            printf(	"  Static fields     -\n");
			for (UINT j=0; j<(dex.DexClassDefs[i].ClassDataOff?dex.DexClasses[i].ClassData->StaticFieldsSize:0); j++)
			{
				printf("    #%d              : (in %s)\n"
					   "      name          : '%s'\n"
					   "      type          : '%s'\n"
					   "      access        : 0x%04x (%s)\n",
					   
					   j, dex.DexClasses[i].Descriptor,
					   dex.DexClasses[i].ClassData->StaticFields[j].Name,
					   dex.DexClasses[i].ClassData->StaticFields[j].Type,
					   dex.DexClasses[i].ClassData->StaticFields[j].AccessFlags, 
					   dex.GetAccessMask(2, dex.DexClasses[i].ClassData->StaticFields[j].AccessFlags)
					   );
			}


            printf(	"  Instance fields   -\n");
			for (UINT j=0; j<(dex.DexClassDefs[i].ClassDataOff?dex.DexClasses[i].ClassData->InstanceFieldsSize:0); j++)
			{
				printf("    #%d              : (in %s)\n"
					   "      name          : '%s'\n"
					   "      type          : '%s'\n"
					   "      access        : 0x%04x (%s)\n",
					   
					   j, dex.DexClasses[i].Descriptor,
					   dex.DexClasses[i].ClassData->InstanceFields[j].Name,
					   dex.DexClasses[i].ClassData->InstanceFields[j].Type,
					   dex.DexClasses[i].ClassData->InstanceFields[j].AccessFlags, 
					   dex.GetAccessMask(2, dex.DexClasses[i].ClassData->InstanceFields[j].AccessFlags)
					   );
			}


            printf(	"  Direct methods    -\n");
            for (UINT j=0; j<(dex.DexClassDefs[i].ClassDataOff?dex.DexClasses[i].ClassData->DirectMethodsSize:0); j++)
            {
                printf ("    #%d              : (in %s)\n"
                        "      name          : '%s'\n"
                        "      type          : '(%s)%s'\n"
                        "      access        : 0x%04x (%s)\n",

                        j, dex.DexClasses[i].Descriptor,
                        dex.DexClasses[i].ClassData->DirectMethods[j].Name,
						dex.DexClasses[i].ClassData->DirectMethods[j].Type, dex.DexClasses[i].ClassData->DirectMethods[j].ProtoType,
						dex.DexClasses[i].ClassData->DirectMethods[j].AccessFlags,  
						dex.GetAccessMask(1, dex.DexClasses[i].ClassData->DirectMethods[j].AccessFlags));

                if (dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea)
                {
                    printf( "      code          -\n"
                            "      registers     : %d\n"
                            "      ins           : %d\n"
                            "      outs          : %d\n"
                            "      insns size    : %d 16-bit code units\n",

						    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->RegistersSize,
						    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->InsSize,
						    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->OutsSize,
						    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->InstBufferSize);


                    printf("%06x:                                        |[%06x] %s.%s:(%s)%s\n", 
                        dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex.DexClasses[i].Descriptor,
                        dex.DexClasses[i].ClassData->DirectMethods[j].Name,
                        dex.DexClasses[i].ClassData->DirectMethods[j].Type,
                        dex.DexClasses[i].ClassData->DirectMethods[j].ProtoType);

                    UINT line = 0;
                    for (UINT k=0; k<dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->InstructionsSize; k++)
                    {
                        printf("%06x: ", dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->Offset & 0x00FFFFFF);
                        for (UINT l=0; l<dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BytesSize; l++)
                        {
                            printf("%02x", dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->Bytes[l]);
                            if ((l+1)%2 == 0) printf(" ");
                        }

                        for (UINT l=0; l<39 - (dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BytesSize*2) - 
                            (dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BytesSize/2) ; l++)
                            printf(" ");

                        printf("|%04x: %s\n", line,
                            dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->Decoded);
                        line += dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Instructions[k]->BytesSize/2;
                    }

				    if (!dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize)
					    printf("      catches       : (none)\n");
				    else
				    {
					    printf("      catches       : %d\n", 
					    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize);

					    for (UINT k=0; k<dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->TriesSize; k++)
					    {
						    printf("        0x%04x - 0x%04x\n", 
							    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].InstructionsStart,
							    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].InstructionsEnd);

                            /*for (UINT l=0; l<(UINT)dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlersSize; l++)
                            {
                                printf("          %s -> 0x%04x\n", 
                                    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Type,
                                    dex.DexClasses[i].ClassData->DirectMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Address);
                            }*/

					//    }
				    //}


				    /*printf(	"      catches       : %s\n"
                            "      positions     :\n" 
                            "	     0x0000 line=36\n"
                            "	     0x0006 line=37\n"
                            "		 0x0009 line=39\n"
                            "      locals        :\n"
						    );*/
            /*    }
                else
                    printf( "      code          : (none)\n");
            }


            
			printf(	"  Virtual methods   -\n");
            for (UINT j=0; j<(dex.DexClassDefs[i].ClassDataOff?dex.DexClasses[i].ClassData->VirtualMethodsSize:0); j++)
            {
                printf ("    #%d              : (in %s)\n"
                        "      name          : '%s'\n"
                        "      type          : '(%s)%s'\n"
                        "      access        : 0x%04x (%s)\n",

                        j, dex.DexClasses[i].Descriptor,
                        dex.DexClasses[i].ClassData->VirtualMethods[j].Name,
						dex.DexClasses[i].ClassData->VirtualMethods[j].Type, dex.DexClasses[i].ClassData->VirtualMethods[j].ProtoType,
						dex.DexClasses[i].ClassData->VirtualMethods[j].AccessFlags,  
						dex.GetAccessMask(1, dex.DexClasses[i].ClassData->VirtualMethods[j].AccessFlags));

                if (dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea)
                {
                    printf( "      code          -\n"
                            "      registers     : %d\n"
                            "      ins           : %d\n"
                            "      outs          : %d\n"
                            "      insns size    : %d 16-bit code units\n",
						    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->RegistersSize,
						    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InsSize,
						    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->OutsSize,
						    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InstBufferSize);
                
                    printf("%06x:                                        |[%06x] %s.%s:(%s)%s\n", 
                        dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Offset & 0x00FFFFFF,
                        dex.DexClasses[i].Descriptor,
                        dex.DexClasses[i].ClassData->VirtualMethods[j].Name,
                        dex.DexClasses[i].ClassData->VirtualMethods[j].Type,
                        dex.DexClasses[i].ClassData->VirtualMethods[j].ProtoType);

                    UINT line = 0;
                    for (UINT k=0; k<dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->InstructionsSize; k++)
                    {
                        printf("%06x: ", dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->Offset & 0x00FFFFFF);
                        for (UINT l=0; l<dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BytesSize; l++)
                        {
                            printf("%02x", dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->Bytes[l]);
                            if ((l+1)%2 == 0) printf(" ");
                        }

                        for (UINT l=0; l<39 - (dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BytesSize*2) - 
                            (dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BytesSize/2) ; l++)
                            printf(" ");

                        printf("|%04x: %s\n", line,
                            dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->Decoded);
                        line += dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Instructions[k]->BytesSize/2;
                    }

				    if (!dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize)
					    printf("      catches       : (none)\n");
				    else
				    {
					    printf("      catches       : %d\n", 
					    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize);

					    for (UINT k=0; k<dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->TriesSize; k++)
					    {
						    printf("        0x%04x - 0x%04x\n", 
							    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].InstructionsStart,
							    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].InstructionsEnd);

                            /*for (UINT l=0; l<(UINT)dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlersSize; l++)
                            {
                                printf("          %s -> 0x%04x\n", 
                                    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Type,
                                    dex.DexClasses[i].ClassData->VirtualMethods[j].CodeArea->Tries[k].CatchHandler->TypeHandlers[l].Address);
                            }*/

					//    }
				    //}


				    /*printf(	"      catches       : %s\n"
                            "      positions     :\n" 
                            "	     0x0000 line=36\n"
                            "	     0x0006 line=37\n"
                            "		 0x0009 line=39\n"
                            "      locals        :\n"
						    );*/
                /*}
                else
                    printf( "      code          : (none)\n");

                }
			

			printf("  source_file_idx   : %d (%s)\n\n", 
                dex.DexClassDefs[i].SourceFileIdx,
                dex.DexClasses[i].SourceFile);
                
        }*/

        system("pause");
    }
    else 
        printf("Unable to load your dex file\n");

    return 0;
}