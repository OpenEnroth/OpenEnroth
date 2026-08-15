#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Logger/LogCategory.h"
#include "Library/Logger/LogEnums.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"
#include "Library/Serialization/Serialization.h"

class TestLogSink : public LogSink {
 public:
    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override {
        messages.emplace_back(message);
    }

    std::vector<std::string> messages;
};

UNIT_TEST(Logger, GlobalLoggerIsAlwaysUsable) {
    // The global logger used to be null until someone constructed a `Logger`, so logging before that point crashed.
    ASSERT_NE(logger, nullptr);
    EXPECT_EQ(logger, detail::fallbackLogger());
    EXPECT_NE(logger->sink(), nullptr); // The fallback logger writes to stderr through a constant-initialized sink.
}

UNIT_TEST(Logger, FallbackLoggerWritesToStderr) {
    ASSERT_EQ(logger, detail::fallbackLogger()); // Otherwise a stray user-created logger is still installed.

    testing::internal::CaptureStderr();
    logger->error("fallback message");
    std::string captured = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(captured.contains("fallback message"));
    EXPECT_TRUE(captured.contains("error"));
}

UNIT_TEST(Logger, UserLoggerReplacesFallbackAndGivesItBack) {
    ASSERT_EQ(logger, detail::fallbackLogger());

    TestLogSink sink;
    std::unique_ptr<Logger> userLogger = std::make_unique<Logger>(LOG_TRACE, &sink);

    EXPECT_EQ(logger, userLogger.get());
    logger->info("routed to the sink");
    ASSERT_EQ(sink.messages.size(), 1);
    EXPECT_EQ(sink.messages[0], "routed to the sink");

    userLogger.reset();

    // Destroying a user-created logger puts the fallback back, so that logging during shutdown still works.
    EXPECT_EQ(logger, detail::fallbackLogger());
}

UNIT_TEST(FallbackLogSink, LevelNamesDontNeedTheSerializationTables) {
    // The fallback sink used to run level names through `toString`. That reads a serialization table that's only
    // built during dynamic initialization, so logging from a static constructor - the very thing the fallback logger
    // exists for - threw instead of logging. Level names have to be available at compile time.
    static_assert(std::string_view(logLevelName(LOG_WARNING)) == "warning");
    static_assert(std::string_view(logLevelName(LOG_NONE)) == "none");
}

UNIT_TEST(FallbackLogSink, LevelNamesMatchSerialization) {
    // `logLevelName` is a second spelling of the level names, and a log line that says `[warn]` while the config file
    // wants `warning` would be a mess. `LogEnums.cpp` builds the table out of `logLevelName` to keep the two
    // together, and this pins that.
    for (LogLevel level : {LOG_NONE, LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_CRITICAL})
        EXPECT_EQ(std::string_view(logLevelName(level)), toString(level));
}
