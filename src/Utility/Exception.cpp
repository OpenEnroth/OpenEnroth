#include "Exception.h"

#include <cassert>
#include <cerrno>
#include <system_error>
#include <string>

void Exception::throwFromErrno(std::string_view arg) {
    assert(errno != 0);

    throw Exception("{}: {}", arg, std::system_category().message(errno));
}

void Exception::throwFromErrc(std::errc error, std::string_view arg) {
    assert(error != std::errc());

    throw Exception("{}: {}", arg, std::make_error_code(error).message());
}
