#pragma once

#include <cassert>

#define Error(...)                                                  \
    do {                                                                    \
        Error_impl_(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
        assert(false);                                                      \
        exit(0);                                                            \
    } while (0)

#define Assert(condition, ...)                                            \
    Assert_impl_(__FILE__, __FUNCTION__, __LINE__, condition, #condition, __VA_ARGS__)

void Error_impl_(const char *filename, const char *functionname,
                 int line, const char *format, ...);

void Assert_impl_(const char *filename, const char *functionname,
                  int line, bool condition, const char *condition_string,
                  const char *format = nullptr, ...);

#ifndef WIN32
#define __debugbreak(...) assert(false);
#endif

