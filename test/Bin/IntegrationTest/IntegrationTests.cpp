#include <string>
#include <string_view>

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileOutputStream.h"

#include "IntegrationTest.h"

constexpr std::string_view configName = "openenroth.ini";

TEST_F(IntegrationTest, Help) {
    ProcessResult result = runOpenEnroth({"--help"});
    EXPECT_TRUE(result.output.contains("--user-path")) << result.output;
}

TEST_F(IntegrationTest, UnknownOption) {
    ProcessResult result = runOpenEnroth({"--no-such-option"});
    EXPECT_NE(result.exitCode, 0);
    EXPECT_TRUE(result.output.contains("--no-such-option")) << result.output;
}

TEST_F(IntegrationTest, Issue1167) {
    // The game ignored openenroth.ini on startup, ran with the defaults and wrote them over the file on exit.
    FileOutputStream(userPath() / configName).write("[debug]\nno_video = true\n[gameplay]\nparty_walk_speed = 400\n"); // The movie player leaks, which fails the run under LeakSanitizer.

    ProcessResult result = runOpenEnroth({"--headless", "--exit-after-start", "--user-path", userPath().toWtf8()});
    EXPECT_EQ(result.exitCode, 0) << result.output;
    EXPECT_TRUE(Blob::fromFile(userPath() / configName).str().contains("party_walk_speed = 400")) << result.output;
}
