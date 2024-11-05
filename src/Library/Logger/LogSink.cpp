#include "LogSink.h"

#include <cassert>
#include <utility>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <spdlog/sinks/android_sink.h> // NOLINT
#include <spdlog/sinks/dist_sink.h> // NOLINT
#include <spdlog/sinks/msvc_sink.h> // NOLINT
#include <spdlog/sinks/stdout_color_sinks.h> // NOLINT

#include "Utility/Win/Unicode.h"

#include "LogEnumFunctions.h"

template<class BaseSink>
class SpdlogSink : public LogSink {
 public:
    template<class... Args>
    explicit SpdlogSink(Args &&... args): _base(std::forward<Args>(args)...) {}

    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override {
        // Note that we don't fill source location.
        _base.log(spdlog::details::log_msg(category.name(), translateLogLevel(level), message));
    }

    BaseSink &base() {
        return _base;
    }

 private:
    BaseSink _base;
};

#ifdef __ANDROID__
class AndroidSinkSt : public LogSink {
 public:
    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override {
        auto pos = _sinkByCategory.find(category.name());
        if (pos == _sinkByCategory.end()) {
            pos = _sinkByCategory.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(category.name()),
                                          std::forward_as_tuple(std::string(category.name()))).first;
        }

        pos->second.log(spdlog::details::log_msg(category.name(), translateLogLevel(level), message));
    }

 private:
    std::unordered_map<std::string_view, spdlog::sinks::android_sink_st> _sinkByCategory;
};
#endif

std::unique_ptr<LogSink> LogSink::createDefaultSink() {
#if defined(__ANDROID__)
    return std::make_unique<AndroidSinkSt>();
#elif defined(_WINDOWS)
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks = {
        std::make_shared<spdlog::sinks::msvc_sink_st>(),
        std::make_shared<spdlog::sinks::stderr_color_sink_st>()
    };
    return std::make_unique<SpdlogSink<spdlog::sinks::dist_sink_st>>(std::move(sinks));
#else
    return std::make_unique<SpdlogSink<spdlog::sinks::stderr_color_sink_st>>();
#endif
}

