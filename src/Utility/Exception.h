#pragma once

#include <stdexcept>
#include <string_view>
#include <utility>

#include "Utility/String/Format.h"

class Exception : public std::runtime_error {
 public:
    template<class... Args>
    explicit Exception(fmt::format_string<Args...> fmt, Args&&... args) : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {}

    [[noreturn]] static void throwFromErrno(std::string_view arg);
    [[noreturn]] static void throwFromErrc(std::errc error, std::string_view arg);
};
