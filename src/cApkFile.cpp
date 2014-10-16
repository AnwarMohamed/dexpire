#include "cApkFile.h"
#include <stdio.h>

cApkFile::cApkFile(CHAR* Filename):
    cFile(Filename)
{
    Files = NULL;
    isReady = BaseAddress && ProcessApk();
}

BOOL cApkFile::ProcessApk()
{
    ZipHandler = OpenZip((PVOID)BaseAddress, FileLength, 0);
    if (!ZipHandler) 
        return FALSE;

    GetZipItem(ZipHandler, -1, &ZipEntry);
    FilesCount = ZipEntry.index;
    Files = new APK_STRUCTURE_FILE[FilesCount];

    for (UINT i=0; i<FilesCount; i++)
    {
        GetZipItem(ZipHandler, i, &ZipEntry);

        Files[i].Name = new CHAR[strlen(ZipEntry.name)+1];
        strcpy_s(Files[i].Name, strlen(ZipEntry.name)+1, ZipEntry.name);

        Files[i].Size = ZipEntry.unc_size;
        Files[i].Buffer = new CHAR[Files[i].Size];

        if (strcmp(Files[i].Name + strlen(Files[i].Name) - 4, ".xml") == 0)
            cBinXMLFile* xml = new cBinXMLFile(Files[i].Buffer, Files[i].Size);

        UnzipItem(ZipHandler, i, Files[i].Buffer, Files[i].Size);
    }

    return TRUE;
}

cApkFile::~cApkFile()
{
    for (UINT i=0; i<FilesCount; i++)
    {
        delete Files[i].Name;
        delete Files[i].Buffer;
    }

    if (Files)
        delete Files;

    if (ZipHandler)
        CloseZip(ZipHandler);
}