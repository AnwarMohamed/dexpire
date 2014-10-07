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