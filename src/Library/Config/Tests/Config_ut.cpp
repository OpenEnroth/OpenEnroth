#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Config/Config.h"
#include "Library/Config/ConfigSection.h"
#include "Library/Config/ConfigEntry.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobOutputStream.h"

// Minimal concrete Config subclass for testing.
struct TestConfig : Config {
    struct TestSection : ConfigSection {
        explicit TestSection(Config *config) : ConfigSection(config, "test") {}

        ConfigEntry<int> value = {this, "value", 42, "Test integer entry."};
        ConfigEntry<bool> flag = {this, "flag", false, "Test boolean entry."};
        ConfigEntry<std::string> name = {this, "name", "default", "Test string entry."};
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

UNIT_TEST_FIXTURE(ConfigTest, SavedValuesAreLoadedBack) {
    // Issue #1167: the game started with default values instead of the ones it had itself written to the ini.
    TestConfig saved;
    saved.test.value.setValue(-13);
    saved.test.flag.setValue(true);
    // Interior '=' and spaces round-trip, the parser splits at the first '=' and trims only the padding.
    saved.test.name.setValue("with spaces and = sign");

    Blob blob;
    BlobOutputStream out(&blob);
    saved.save(&out);
    out.close();

    TestConfig loaded;
    MemoryInputStream in(blob.data(), blob.size());
    EXPECT_NO_THROW(loaded.load(&in));
    EXPECT_EQ(loaded.test.value.value(), -13);
    EXPECT_EQ(loaded.test.flag.value(), true);
    EXPECT_EQ(loaded.test.name.value(), "with spaces and = sign");
}
