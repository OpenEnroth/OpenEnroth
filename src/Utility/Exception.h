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

    /**
     * Same as the above, but for an `errno` captured earlier - needed when cleanup would clobber it.
     *
     * @param error                     `errno` value.
     * @param arg                       Context to prepend to the error message, usually a file path.
     * @throws Exception                Always.
     */
    [[noreturn]] static void throwFromErrno(int error, std::string_view arg);
    [[noreturn]] static void throwFromErrc(std::errc error, std::string_view arg);
};
