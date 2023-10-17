#pragma once

#include <memory>

#include <CLI/CLI.hpp>

/**
 * Utility class that converts program arguments to utf-8 on Windows, and does nothing on POSIX.
 *
 * Use it like this:
 * ```
 * int main(int argc, char **argv) {
 *     try {
 *         CliUtf8fy _(argc, argv);
 *         // Use argc & argv here.
 *     } catch (...) {
 *         // Your error processing here.
 *     }
 * }
 * ```
 *
 * Note that while `CliUtf8fy` constructor shouldn't normally throw, it can theoretically throw.
 *
 * Also note that `CliUtf8fy` is only needed if the program isn't using `platformMain`.
 */
class CliUtf8fy {
 public:
    CliUtf8fy(int /*argc*/, char **&argv) {
        _app = std::make_unique<CLI::App>();
        argv = _app->ensure_utf8(argv);
    }

 private:
    std::unique_ptr<CLI::App> _app;
};
