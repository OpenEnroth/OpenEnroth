#include "Exception.h"

#include <cassert>
#include <cerrno>
#include <system_error>

void Exception::throwFromErrno(std::string_view arg) {
    throwFromErrno(errno, arg);
}

void Exception::throwFromErrno(int error, std::string_view arg) {
    assert(error != 0);

    // `errno` belongs to `generic_category`. `system_category` is the same thing on POSIX, but means Win32 codes on
    // Windows, where it would describe a completely unrelated error.
    throw Exception("{}: {}", arg, std::generic_category().message(error));
}

void Exception::throwFromErrc(std::errc error, std::string_view arg) {
    assert(error != std::errc());

    throw Exception("{}: {}", arg, std::make_error_code(error).message());
}
