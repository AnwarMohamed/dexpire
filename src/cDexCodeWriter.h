/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
 *
 */

#pragma once
#include "dexpire.h"
#include <stdio.h>

class DLLEXPORT cDexCodeWriter
{

public:
    cDexCodeWriter(UINT IndentSize=4);
    ~cDexCodeWriter();

    void IndentForward(UINT IndentSize=0);
    void IndentBackward(UINT IndentSize=0);

    void Print(CHAR* Arguments);
    void PrintLine(CHAR* Arguments);
    void PrintStartLine(CHAR* Arguments);
    void PrintStartEndLine(CHAR* Arguments);
private:
    UINT IndentSize;
    UINT IndentLevel;

    void cDexCodeWriter::PrintIndentation();
};