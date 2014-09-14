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

#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport) 
#else
#define DLLEXPORT
#endif
#endif