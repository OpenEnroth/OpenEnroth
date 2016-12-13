#pragma once
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS

#define Error(format, ...)      do {Error_impl_(__FILE__, __FUNCTION__, __LINE__, format, __VA_ARGS__); __debugbreak(); exit(0); } while (0)
#define Assert(condition, ...)  Assert_impl_(__FILE__, __FUNCTION__, __LINE__, condition, #condition,  __VA_ARGS__)



#include <stdarg.h>
#include <stdio.h>
inline __declspec(noreturn) void Error_impl_(const char *filename, const char *functionname, int line,
                                             const char *format, ...)
{
  va_list va;
  va_start(va, format);
  {
    char header[4096];
    sprintf_s(header, "Error in %s: %u\n\t%s\n\n", filename, line, functionname);

    char msg_body[8192];
    vsprintf_s(msg_body, format, va);

    wchar_t msg[sizeof(header) + sizeof(msg_body)];
    swprintf(msg, 8192, L"%S %S", header, msg_body);

    extern void MsgBox(const wchar_t *, const wchar_t *);
    MsgBox(msg, L"Error");
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
    sprintf_s(header, "Assertion in %s: %u\n\t%s:\n%s\n\n", filename, line, functionname, condition_string);

    char msg_body[8192];
    vsprintf_s(msg_body, format, va);
    
    wchar_t msg[sizeof(header) + sizeof(msg_body)];
    if (format)
      swprintf(msg, (sizeof(header) + sizeof(msg_body)), L"%S %S", header, msg_body);
    else
      swprintf(msg, (sizeof(header) + sizeof(msg_body)), L"%S", header);

    extern void MsgBox(const wchar_t *, const wchar_t *);
    MsgBox(msg, L"Assertion");
  }
  va_end(va);

  __debugbreak();
}