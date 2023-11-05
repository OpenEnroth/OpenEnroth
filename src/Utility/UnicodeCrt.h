#pragma once

#include <vector>
#include <string>

/**
 * Utility class that turns on UTF-8 for most of CRT, and converts command-line arguments to UTF-8. This is really only
 * needed on Windows, and this class does nothing on POSIX.
 *
 * Use it like this:
 * ```
 * int main(int argc, char **argv) {
 *     try {
 *         UnicodeCrt _(argc, argv);
 *         // Use argc & argv here.
 *     } catch (...) {
 *         // Your error processing here.
 *     }
 * }
 * ```
 *
 * Note that for this to work on older Windows versions, CRT should be statically linked. This is how OE releases
 * are built right now.
 *
 * Also note that while `UnicodeCrt` constructor shouldn't normally throw, it can throw in theory.
 *
 * @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#utf-8-support
 */
class UnicodeCrt {
 public:
    UnicodeCrt(int &argc, char **&argv);

    /**
     * @return                          Whether a `UnicodeCrt` was created, and thus CRT now uses UTF-8.
     */
    static bool isInitialized();

 private:
    std::vector<char *> _argv;
    std::vector<std::string> _storage;
};
