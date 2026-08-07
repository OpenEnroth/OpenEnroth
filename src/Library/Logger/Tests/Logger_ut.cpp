#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Logger/LogCategory.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"

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
