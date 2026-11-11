#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "Library/Lod/LodReader.h"
#include "Library/Lod/LodWriter.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Ascii.h"
#include "Utility/System/Fs.h"

#include "IntegrationTest.h"

constexpr std::string_view configName = "openenroth.ini";

static Path mirrorWithSymlinks(const Path &src, const Path &dst, std::string_view excludedName) {
    // Symlinks everything in src into dst except the entry named excludedName in any case, and returns that entry's path.
    Path excluded;
    fs::mkdirs(dst);
    for (const DirectoryEntry &entry : fs::ls(src)) {
        if (ascii::noCaseEquals(entry.name, excludedName)) {
            excluded = src / entry.name;
        } else if (entry.type == FILE_DIRECTORY) {
            std::filesystem::create_directory_symlink((src / entry.name).native(), (dst / entry.name).native());
        } else {
            std::filesystem::create_symlink((src / entry.name).native(), (dst / entry.name).native());
        }
    }
    ASSERT_FALSE(excluded.isEmpty()) << excludedName << " not found in " << src.str();
    return excluded;
}

TEST_F(IntegrationTest, Issue1167a) {
    // Startup looked for openenroth.ini in the wrong place, a first start has to create it in the user folder.
    ProcessResult result = runOpenEnroth({"--headless", "--exit-after-start", "--user-path", userPath().str()});
    EXPECT_EQ(result.exitCode, 0) << result.stdErr;
    EXPECT_TRUE(fs::exists(userPath() / configName)) << result.stdErr;
}

TEST_F(IntegrationTest, Issue1167b) {
    // The game ignored openenroth.ini on startup, ran with the defaults and wrote them over the file on exit.
    FileOutputStream(userPath() / configName).write("[gameplay]\nparty_walk_speed = 400\n");

    ProcessResult result = runOpenEnroth({"--headless", "--exit-after-start", "--user-path", userPath().str()});
    EXPECT_EQ(result.exitCode, 0) << result.stdErr;
    EXPECT_TRUE(Blob::fromFile(userPath() / configName).str().contains("party_walk_speed = 400")) << result.stdErr;
}

TEST_F(IntegrationTest, Issue2983) {
    // The game crashed at startup on text tables ending in a DOS end-of-file byte, as the Czech fan translation ships them.
    const char *mm7Path = std::getenv("OPENENROTH_MM7_PATH");
    ASSERT_NE(mm7Path, nullptr);

    Path dataPath = userPath() / "mm7";
    Path srcDataFolder = mirrorWithSymlinks(Path(mm7Path), dataPath, "data");
    Path srcEventsLod = mirrorWithSymlinks(srcDataFolder, dataPath / "data", "events.lod");

    std::unordered_set<std::string> tables = {"2devents.txt", "history.txt", "items.txt", "npctext.txt", "potion.txt", "potnotes.txt", "spells.txt"};
    LodReader reader(srcEventsLod);
    LodWriter writer(dataPath / "data" / "events.lod", reader.info());
    for (const std::string &name : reader.ls()) {
        Blob data = reader.read(name);
        if (tables.contains(ascii::toLower(name)))
            data = Blob::fromString(std::string(lod::decodeMaybeCompressed(data).str()) + '\x1A');
        writer.write(name, std::move(data));
    }
    writer.close();

    ProcessResult result = runOpenEnroth({"--headless", "--exit-after-start", "--data-path", dataPath.str(), "--user-path", userPath().str()});
    EXPECT_EQ(result.exitCode, 0) << result.stdErr;
}
