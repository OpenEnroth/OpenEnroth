#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "Engine/Log.h"

#include "Platform/Api.h"

bool Log::Initialize() { return this->initialized = OS_OpenConsole(); }

void Log::Info(const wchar_t *pFormat, ...) {
    if (this->initialized) {
        va_list args;
        wchar_t pMsg[8192];

        va_start(args, pFormat);
        vswprintf_s(pMsg, 8192, pFormat, args);
        va_end(args);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("[%04d/%02d/%02d %02d:%02d:%02d] %S\n", tm.tm_year + 1900,
               tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
               pMsg);
    }
}

void Log::Warning(const wchar_t *pFormat, ...) {
    if (this->initialized) {
        va_list args;
        wchar_t pMsg[8192];

        va_start(args, pFormat);
        vswprintf_s(pMsg, 8192, pFormat, args);
        va_end(args);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("[%04d/%02d/%02d %02d:%02d:%02d] %S\n", tm.tm_year + 1900,
               tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
               pMsg);
    }
}

//----- (004BE386) --------------------------------------------------------
void log_error(const char *pMessage) {
    const char *v1;  // edi@1
    FILE *f;         // eax@1
    FILE *v3;        // esi@1

    v1 = pMessage;
    f = fopen("errorlog.txt", "a");
    v3 = f;
    if (f) {
        fprintf(f, "%s\n", v1);
        fclose(v3);
        fflush(v3);
    }
}
