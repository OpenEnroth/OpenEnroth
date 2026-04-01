#include "Testing/Unit/UnitTest.h"

#include "Library/Config/Config.h"
#include "Library/Config/ConfigSection.h"
#include "Library/Config/ConfigEntry.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"

#include "Utility/Streams/MemoryInputStream.h"

// Minimal concrete Config subclass for testing.
struct TestConfig : Config {
    struct TestSection : ConfigSection {
        explicit TestSection(Config *config) : ConfigSection(config, "test") {}

        ConfigEntry<int> value = {this, "value", 42, "Test integer entry."};
    } test{this};
};

// Null log sink that discards all messages.
class NullLogSink : public LogSink {
 public:
    virtual void write(const LogCategory &, LogLevel, std::string_view) override {}
};

// Fixture that initializes the global logger singleton required by Config::load.
class ConfigTest : public testing::Test {
 protected:
    NullLogSink _sink;
    Logger _logger{LOG_TRACE, &_sink};
};

UNIT_TEST_FIXTURE(ConfigTest, InvalidValueIsIgnoredAndPreviousValueKept) {
    // Loading an invalid value should log a warning and leave the entry unchanged.
    TestConfig config;
    config.test.value.setValue(99);

    std::string_view ini = "[test]\nvalue = not_a_number\n";
    MemoryInputStream stream(ini.data(), ini.size());
    EXPECT_NO_THROW(config.load(&stream));
    EXPECT_EQ(config.test.value.value(), 99); // Previous value preserved, not default.
}

UNIT_TEST_FIXTURE(ConfigTest, ValidValueIsLoaded) {
    TestConfig config;

    std::string_view ini = "[test]\nvalue = 7\n";
    MemoryInputStream stream(ini.data(), ini.size());
    EXPECT_NO_THROW(config.load(&stream));
    EXPECT_EQ(config.test.value.value(), 7);
}
