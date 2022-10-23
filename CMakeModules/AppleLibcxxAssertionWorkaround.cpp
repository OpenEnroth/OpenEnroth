#include <cstring>
#include <string>

#ifdef _LIBCPP_DEBUG

static void debug_handler(const std::__libcpp_debug_info &info) {
    const size_t bufsize = 1024;
    char buf[bufsize] = {};
    char *pos = buf;
    char *end = buf + bufsize - 1;

    pos = stpncpy(pos, info.__pred_, end - pos);
    pos = stpncpy(pos, " (", end - pos);
    pos = stpncpy(pos, info.__msg_, end - pos);
    pos = stpncpy(pos, ")", end - pos);

    __assert_rtn("", info.__file_, info.__line_, buf);
}

std::__libcpp_debug_function_type std::__libcpp_debug_function = &debug_handler;

#endif
