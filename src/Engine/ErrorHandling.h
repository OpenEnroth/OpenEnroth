#pragma once

#include <cassert>
#include <functional>
#include <string>

#define Error(...)                                                                                                      \
    do {                                                                                                                \
        Error_impl_(__FILE__, __FUNCTION__, __LINE__ __VA_OPT__(,) __VA_ARGS__); /* NOLINT */                           \
    } while (0)

[[noreturn]] void Error_impl_(const char *filename, const char *functionname, int line, const char *format, ...);

#ifndef _WINDOWS
#   define __debugbreak(...) assert(false);
#endif

using ErrorHandlerFunction = std::function<void(const std::string &/* title */, const std::string &/* message */)>;

void setErrorHandler(ErrorHandlerFunction handler);
