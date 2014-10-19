/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#pragma once
#include "cFile.h"
#include "unzip.h"
#include "cBinXMLFile.h"

STRUCT APK_STRUCTURE_FILE
{
    CHAR* Name;
    UINT Size;
    CHAR* Buffer;
};

class DLLEXPORT cApkFile: public cFile
{
public:
    cApkFile(CHAR* Filename);
    cApkFile(CHAR* Buffer, DWORD Size);
    ~cApkFile(void);

    BOOL isReady;
    APK_STRUCTURE_FILE* Files;
    UINT FilesCount;

private:
    HZIP ZipHandler;
    ZIPENTRY ZipEntry;
    BOOL ProcessApk();
};


