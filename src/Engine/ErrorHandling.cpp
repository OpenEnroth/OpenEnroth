#include "Engine/ErrorHandling.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <sstream>

#include <Platform/Platform.h>
#include <Engine/EngineGlobals.h>

void Error_impl_(const char *filename, const char *functionname,
                 int line, const char *format, ...) {
    char msg_body[8192] = { 0 };
    if (format != nullptr) {
        va_list va;
        va_start(va, format);
        vsnprintf(msg_body, 8192, format, va);
        va_end(va);
    }

    std::stringstream out;
    out << "Error in " << filename << ": ";
    out << line << "\n\t" << functionname << "\n\n";
    if (strlen(msg_body) > 0) {
        out << "\n\n" << msg_body;
    }

    if (platform)
        platform->ShowMessageBox(out.str(), "Error");
}

void Assert_impl_(const char *filename, const char *functionname,
                  int line, bool condition, const char *condition_string,
                  const char *format, ...) {
    if (condition) return;

    char msg_body[8192] = { 0 };
    if (format != nullptr) {
        va_list va;
        va_start(va, format);
        vsnprintf(msg_body, 8192, format, va);
        va_end(va);
    }

    std::stringstream out;
    out << "Assertion in " << filename << ": ";
    out << line << "\n\t" << functionname << "\n\n";
    if (strlen(msg_body) > 0) {
        out << "\n\n" << msg_body;
    }

    if (platform)
        platform->ShowMessageBox(out.str(), "Assertion");

    assert(false);
}
