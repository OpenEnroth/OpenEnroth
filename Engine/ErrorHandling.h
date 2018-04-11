#pragma once

#include <cstdarg>
#include <cstdio>
#include <cassert>

#define Error(format, ...)      do {Error_impl_(__FILE__, __FUNCTION__, __LINE__, format, __VA_ARGS__); assert(false); exit(0); } while (0)
#define Assert(condition, ...)  Assert_impl_(__FILE__, __FUNCTION__, __LINE__, condition, #condition,  __VA_ARGS__)

inline void Error_impl_(const char *filename, const char *functionname, int line,
    const char *format, ...)
{
    va_list va;
    va_start(va, format);
    {
        char header[4096];
        sprintf(header, "Error in %s: %u\n\t%s\n\n", filename, line, functionname);

        char msg_body[8192];
        vsprintf(msg_body, format, va);

        wchar_t msg[sizeof(header) + sizeof(msg_body)];
        swprintf(msg, 8192, L"%S %S", header, msg_body);

        extern void OS_MsgBox(const wchar_t *, const wchar_t *);
        OS_MsgBox(msg, L"Error");
    }
    va_end(va);
}


inline void Assert_impl_(const char *filename, const char *functionname, int line,
    bool condition, const char *condition_string, const char *format = nullptr, ...)
{
    if (condition)
        return;

    va_list va;
    va_start(va, format);
    {
        char header[4096];
        sprintf(header, "Assertion in %s: %u\n\t%s:\n%s\n\n", filename, line, functionname, condition_string);

        char msg_body[8192];
        vsprintf(msg_body, format, va);

        wchar_t msg[sizeof(header) + sizeof(msg_body)];
        if (format)
            swprintf(msg, (sizeof(header) + sizeof(msg_body)), L"%S %S", header, msg_body);
        else
            swprintf(msg, (sizeof(header) + sizeof(msg_body)), L"%S", header);

        extern void OS_MsgBox(const wchar_t *, const wchar_t *);
        OS_MsgBox(msg, L"Assertion");
    }
    va_end(va);

    assert(false);
}