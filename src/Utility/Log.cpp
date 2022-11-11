#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <cassert>

#include "Log.h"

Log::Log(FILE *infoFile, FILE *warningFile): infoFile_(infoFile), warningFile_(warningFile) {
    assert(infoFile);
    assert(warningFile);
}

void Log::Info(const char *pFormat, ...) {
    va_list args;
    char pMsg[8192];

    va_start(args, pFormat);
    vsnprintf(pMsg, 8192, pFormat, args);
    va_end(args);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(infoFile_, "[%04d/%02d/%02d %02d:%02d:%02d] %s\n", tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
           pMsg);
}

void Log::Warning(const char *pFormat, ...) {
    va_list args;
    char pMsg[8192];

    va_start(args, pFormat);
    vsnprintf(pMsg, 8192, pFormat, args);
    va_end(args);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(warningFile_, "[%04d/%02d/%02d %02d:%02d:%02d] %s\n", tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
           pMsg);
}
