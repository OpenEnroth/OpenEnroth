#include "Exception.h"

#include <cassert>
#include <cerrno>
#include <system_error>

void Exception::throwFromErrno(std::string_view arg) {
    throwFromErrno(errno, arg);
}

void Exception::throwFromErrno(int error, std::string_view arg) {
    assert(error != 0);

    // Note that `errno` values belong to `std::generic_category`. `std::system_category` is the native OS error
    // category, which is the same thing on POSIX, but means Win32 error codes on Windows - so using it here would
    // produce a message for a completely unrelated error there.
    throw Exception("{}: {}", arg, std::generic_category().message(error));
}

void Exception::throwFromErrc(std::errc error, std::string_view arg) {
    assert(error != std::errc());

    throw Exception("{}: {}", arg, std::make_error_code(error).message());
}
