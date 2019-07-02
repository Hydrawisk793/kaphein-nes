#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "kaphein/nes/debug.h"

void
outputDebugString(
    const char * s
    , ...
)
{
    static char debugText[256];
    va_list vaList;

    va_start(vaList, s);

    vsprintf(debugText, s, vaList);

    va_end(vaList);

    OutputDebugString(debugText);
 }
