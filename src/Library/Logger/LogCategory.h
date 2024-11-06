#pragma once

#include <string_view>
#include <optional>
#include <vector>

#include "LogEnums.h"

class LogSource;

/**
 * Log category, usually used to differentiate logging calls from different subsystems.
 *
 * Intended usage is to just create a `static` instance in a `.cpp` file, and use it when logging.
 *
 * Log category can be associated with a `LogSource`, which will then be used when changing per-category log levels.
 */
class LogCategory {
 public:
    /**
     * Creates and registers a new log category instance.
     *
     * @param name                      Name of the log category. `LogCategory` doesn't copy the provided string,
     *                                  so the user is expected to pass a string constant. Different `LogCategory`
     *                                  instances are expected to have different names, and the constructor will
     *                                  assert if this is not satisfied.
     * @param source                    Log source, if any. `LogCategory` doesn't take ownership of the provided
     *                                  `LogSource`, and thus lifetime management is up to the user. The best practice
     *                                  is to create both the `LogSource` and `LogCategory` as global variables.
     */
    explicit LogCategory(std::string_view name, LogSource *source = nullptr);
    ~LogCategory();

    // LogCategory is non-movable & non-copyable.
    LogCategory(const LogCategory &) = delete;
    LogCategory(LogCategory &&) = default;

    [[nodiscard]] std::string_view name() const {
        return _name;
    }

    [[nodiscard]] LogSource *source() const {
        return _source;
    }

    static std::vector<LogCategory *> instances();

 private:
    friend class Logger;

 private:
    std::string_view _name;
    LogSource *_source = nullptr;

    // Storing log level here is an implementation detail - it makes it possible to check the log level inside a
    // logging call in just two memory reads.
    std::optional<LogLevel> _level;

    // Same as above, but we store `detail::LOG_NONE_BARRIER` instead of `LOG_NONE` here so that when we don't read the
    // docs and try to log at `LOG_NONE`, the message is always ignored.
    std::optional<int> _adjustedLevel;
};
