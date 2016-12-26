#include <stdio.h>
#include <stdarg.h>

#include "Engine/Log.h"

#include "Platform/Api.h"

void Log::Initialize()
{
    OS_OpenConsole();
}


void Log::Warning(const wchar_t *pFormat, ...)
{
    va_list args;
    wchar_t pMsg[8192];

    va_start(args, pFormat);
    vswprintf_s(pMsg, 8192, pFormat, args);
    va_end(args);

    printf("%S\n", pMsg);
}

//----- (004BE386) --------------------------------------------------------
void log_error(const char *pMessage)
{
    const char *v1; // edi@1
    FILE *f; // eax@1
    FILE *v3; // esi@1

    v1 = pMessage;
    f = fopen("errorlog.txt", "a");
    v3 = f;
    if (f)
    {
        fprintf(f, "%s\n", v1);
        fclose(v3);
        fflush(v3);
    }
}