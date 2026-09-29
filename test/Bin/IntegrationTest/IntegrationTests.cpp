#include <filesystem>
#include <string>
#include <string_view>

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileOutputStream.h"

#include "IntegrationTest.h"

constexpr std::string_view configName = "openenroth.ini";

TEST_F(IntegrationTest, Issue1167a) {
    // Startup looked for openenroth.ini in the wrong place, a first start has to create it in the user folder.
    ProcessResult result = runOpenEnroth({"--headless", "--exit-after-start", "--user-path", userPath().toWtf8()});
    EXPECT_EQ(result.exitCode, 0) << result.output;
    EXPECT_TRUE(std::filesystem::exists((userPath() / configName).toStdPath())) << result.output;
}

TEST_F(IntegrationTest, Issue1167b) {
    // The game ignored openenroth.ini on startup, ran with the defaults and wrote them over the file on exit.
    FileOutputStream(userPath() / configName).write("[gameplay]\nparty_walk_speed = 400\n");

    ProcessResult result = runOpenEnroth({"--headless", "--exit-after-start", "--user-path", userPath().toWtf8()});
    EXPECT_EQ(result.exitCode, 0) << result.output;
    EXPECT_TRUE(Blob::fromFile(userPath() / configName).str().contains("party_walk_speed = 400")) << result.output;
}
