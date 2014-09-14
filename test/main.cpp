#include "..\src\cDexFile.h"
#include <stdio.h>

int main()
{
    cDexFile dex("classes.dex");
    printf("Processing '%s'\n", dex.Filename);

    if (dex.isReady)
    {
        printf("Opened '%s', DEX version '%s'\n", dex.Filename, dex.DexVersion);

        for (UINT i=5; i<6/*dex.nClassDefinitions*/; i++)
        {
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
                    dex.DexClasses[i].ClassData->StaticFieldsSize,
                    dex.DexClasses[i].ClassData->InstanceFieldsSize,
                    dex.DexClasses[i].ClassData->DirectMethodsSize,
                    dex.DexClasses[i].ClassData->VirtualMethodsSize);

            printf(	"Class #%d            -\n"
                    "  Class descriptor  : '%s'\n"
                    "  Access flags      : 0x%04x (%s)\n"
                    "  Superclass        : '%s'\n",
                    
                    i,
                    dex.DexClasses[i].Descriptor,
                    dex.DexClasses[i].AccessFlags, dex.GetAccessMask(0, dex.DexClasses[i].AccessFlags),
                    dex.DexClasses[i].SuperClass);


            printf(	"  Interfaces        -\n");
			for (UINT j=0; j<dex.DexClasses[i].ClassData->InterfacesSize; j++)
			{
				printf(	"    #%d              : '%s'\n", 
						j, dex.DexClasses[i].ClassData->Interfaces[j]);
			}


            printf(	"  Static fields     -\n");
			for (UINT j=0; j<dex.DexClasses[i].ClassData->StaticFieldsSize; j++)
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
			for (UINT j=0; j<dex.DexClasses[i].ClassData->InstanceFieldsSize; j++)
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
            for (UINT j=0; j<dex.DexClasses[i].ClassData->DirectMethodsSize; j++)
            {
                printf ("    #%d              : (in %s)\n"
                        "      name          : '%s'\n"
                        "      type          : '(%s)%s'\n"
                        "      access        : 0x%04x (%s)\n"
                        "      code          -\n"
                        "      registers     : 3\n"
                        "      ins           : 1\n"
                        "      outs          : 2\n"
                        "      insns size    : 15 16-bit code units\n"
                        "      catches       : (none)\n"
                        "      positions     :\n" 
                        "	     0x0000 line=36\n"
                        "	     0x0006 line=37\n"
                        "		 0x0009 line=39\n"
                        "      locals        :\n",

                        j, dex.DexClasses[i].Descriptor,
                        dex.DexClasses[i].ClassData->DirectMethods[j].Name,
						/*dex.DexClasses[i].ClassData->DirectMethods[j].Type*/"", dex.DexClasses[i].ClassData->DirectMethods[j].ProtoType,
						dex.DexClasses[i].ClassData->DirectMethods[j].AccessFlags,  
						dex.GetAccessMask(1, dex.DexClasses[i].ClassData->DirectMethods[j].AccessFlags));
            }


			printf(	"  Virtual methods   -\n");
			for (UINT j=0; j<dex.DexClasses[i].ClassData->VirtualMethodsSize; j++)
            {
                printf ("    #%d              : (in %s)\n"
                        "      name          : '%s'\n"
                        "      type          : '(%s)%s'\n"
                        "      access        : 0x%04x (%s)\n"
                        "      code          -\n"
                        "      registers     : 3\n"
                        "      ins           : 1\n"
                        "      outs          : 2\n"
                        "      insns size    : 15 16-bit code units\n"
                        "      catches       : (none)\n"
                        "      positions     :\n" 
                        "	     0x0000 line=36\n"
                        "	     0x0006 line=37\n"
                        "		 0x0009 line=39\n"
                        "      locals        :\n",

                        j, dex.DexClasses[i].Descriptor,
                        dex.DexClasses[i].ClassData->VirtualMethods[j].Name,
						/*dex.DexClasses[i].ClassData->VirtualMethods[j].Type*/"", dex.DexClasses[i].ClassData->VirtualMethods[j].ProtoType,
						dex.DexClasses[i].ClassData->VirtualMethods[j].AccessFlags,  
						dex.GetAccessMask(1, dex.DexClasses[i].ClassData->VirtualMethods[j].AccessFlags));
            }

			printf("  source_file_idx   : %d (SourceFile)\n\n", dex.DexClassDefs[i].SourceFileIdx);
        }

        system("pause");
    }
    else 
        printf("Unable to load your dex file\n");

    return 0;
}