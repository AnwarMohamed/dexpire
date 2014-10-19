/*
 *
 *  Copyright (C) 2014  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE.txt', which is part of this source code package.
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