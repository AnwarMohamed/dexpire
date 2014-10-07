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

#include "cDexCodeWriter.h"

cDexCodeWriter::cDexCodeWriter(
    UINT IndentSize
    )
{
    this->IndentSize = IndentSize;
    this->IndentLevel = 0;
}

void cDexCodeWriter::PrintStartLine(
    CHAR* Arguments
    )
{
    PrintIndentation();
    Print(Arguments);
}

void cDexCodeWriter::PrintStartEndLine(
    CHAR* Arguments
    )
{
    PrintIndentation();
    PrintLine(Arguments);

}

void cDexCodeWriter::PrintLine(
    CHAR* Arguments
    )
{
    Print(Arguments);
    printf("\n");
}

void cDexCodeWriter::Print(
    CHAR* Arguments
    )
{
    va_list ArgPtr;
    va_start(ArgPtr, Arguments);
    va_end(ArgPtr);

    printf("%s", ArgPtr);
}

void cDexCodeWriter::PrintIndentation()
{
    for (UINT i=0; i<this->IndentLevel; i++)
        printf(" ");
}

void cDexCodeWriter::IndentForward(
    UINT IndentSize
    ) 
{ 
    this->IndentLevel += IndentSize? IndentSize: this->IndentSize; 
    PrintIndentation();
}

void cDexCodeWriter::IndentBackward(
    UINT IndentSize
    ) 
{ 
    this->IndentLevel -= IndentSize? IndentSize: this->IndentSize;
    PrintIndentation();
}