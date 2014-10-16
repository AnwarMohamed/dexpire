#include "cBinXMLFile.h"

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
    RESOURCE_HEADER* ResourceHeader;
    RESOURCE_TREE_HEADER* ResourceTreeHeader;
    RESOURCE_STRING_POOL_HEADER* ResourceStringPoolHeader;

    for (UINT i=sizeof(RESOURCE_HEADER); i<FileLength;)
    {
        ResourceTreeHeader = (RESOURCE_TREE_HEADER*)(BaseAddress + i);
        
                
        i+= ResourceTreeHeader->Header.Size;
    }

    return TRUE;
}

cBinXMLFile::~cBinXMLFile()
{
}