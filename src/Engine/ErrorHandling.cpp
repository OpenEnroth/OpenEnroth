#include "ErrorHandling.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <utility>

#include "Utility/Format.h"

[[noreturn]] static void defaultErrorHandler(const std::string &title, const std::string &message) {
    fmt::println(stderr, "{}", title);
    fmt::println(stderr, "{}", message);
    fmt::println(stderr, "Exiting...");
    assert(false);
    exit(1); // TODO(captainurist): Redo. We should throw instead.
}

ErrorHandlerFunction globalErrorHandler;

void Error_impl_(const char *filename, const char *functionname, int line, const char *format, ...) {
    char msg_body[8192] = { 0 };
    if (format != nullptr) {
        va_list va;
        va_start(va, format);
        vsnprintf(msg_body, 8192, format, va);
        va_end(va);
    }

    std::stringstream out;
    out << "Error in " << filename << ": ";
    out << line << "\n\t" << functionname << "\n\n";
    if (strlen(msg_body) > 0) {
        out << "\n\n" << msg_body;
    }

    if (globalErrorHandler)
        globalErrorHandler("Error", out.str());
    defaultErrorHandler("Error", out.str()); // Global handler shouldn't return.
}

void setErrorHandler(ErrorHandlerFunction handler) {
    globalErrorHandler = std::move(handler);
}
