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

#ifdef _WIN32
#pragma warning (disable : 4251)
#include <Windows.h>
#else
#define BOOL     bool
#define UCHAR    unsigned char
#define CHAR     char
#define UINT     unsigned int
#define USHORT   unsigned short
#define ULONG    unsigned long
#define DWORD    ULONG
#define WORD     UINT
#define INT      int
#define ULONGLONG unsigned long long
#define FALSE    0
#define TRUE     1
#endif

#define STRUCT struct

#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport) 
#else
#define DLLEXPORT
#endif
#endif