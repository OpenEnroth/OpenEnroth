#include <filesystem>
#include <fstream>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Application/Startup/PathResolver.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Utility/Memory/Blob.h"

// detectGameType is the single engine-wide game-version detector, both validateDataPath and GameStarter go through it.

namespace {

class TemporaryDataDir {
 public:
    explicit TemporaryDataDir(std::string_view name) : _path(std::filesystem::temp_directory_path() / name) {
        std::filesystem::remove_all(_path);
        std::filesystem::create_directories(_path);
    }

    ~TemporaryDataDir() {
        std::filesystem::remove_all(_path);
    }

    void touch(std::string_view relativePath) const {
        std::filesystem::path filePath = _path / relativePath;
        std::filesystem::create_directories(filePath.parent_path());
        std::ofstream(filePath).close();
    }

    std::string path() const {
        return _path.generic_string();
    }

 private:
    std::filesystem::path _path;
};

} // namespace

UNIT_TEST(PathResolver, DetectGameTypeMm8DataPath) {
    // MM8 installs ship `anims/magicdod.vid`, which MM7 installs don't have.
    MemoryFileSystem fs("test");
    fs.write("anims/magicdod.vid", Blob());

    EXPECT_EQ(detectGameType(fs), GAME_TYPE_MM8);
}

UNIT_TEST(PathResolver, DetectGameTypeMm7DataPath) {
    MemoryFileSystem fs("test");
    fs.write("anims/magic7.vid", Blob());
    fs.write("anims/might7.vid", Blob());

    EXPECT_EQ(detectGameType(fs), GAME_TYPE_MM7);
}

UNIT_TEST(PathResolver, DetectGameTypeDefaultsToMm7WhenAmbiguous) {
    // MM7 is the default whenever detection is ambiguous (e.g. an empty/unknown folder), so that unrecognized
    // data paths keep behaving like they did before this seam existed.
    MemoryFileSystem fs("test");

    EXPECT_EQ(detectGameType(fs), GAME_TYPE_MM7);
}

UNIT_TEST(PathResolver, DetectGameTypeAndValidateDataPathAgreeForMm8DataPath) {
    TemporaryDataDir dir("OpenEnroth_PathResolver_ut_mm8");
    dir.touch("anims/magicdod.vid");
    dir.touch("anims/mightdod.vid");
    dir.touch("data/bitmaps.lod");
    dir.touch("data/games.lod");
    dir.touch("data/icons.lod");
    dir.touch("data/sprites.lod");

    EXPECT_EQ(detectGameType(Path(dir.path())), GAME_TYPE_MM8);

    std::string missingFile;
    EXPECT_TRUE(validateDataPath(Path(dir.path()), &missingFile));
}

UNIT_TEST(PathResolver, DetectGameTypeAndValidateDataPathAgreeForMm7DataPath) {
    TemporaryDataDir dir("OpenEnroth_PathResolver_ut_mm7");
    dir.touch("anims/magic7.vid");
    dir.touch("anims/might7.vid");
    dir.touch("data/bitmaps.lod");
    dir.touch("data/events.lod");
    dir.touch("data/games.lod");
    dir.touch("data/icons.lod");
    dir.touch("data/sprites.lod");
    dir.touch("sounds/audio.snd");

    EXPECT_EQ(detectGameType(Path(dir.path())), GAME_TYPE_MM7);

    std::string missingFile;
    EXPECT_TRUE(validateDataPath(Path(dir.path()), &missingFile));
}

UNIT_TEST(PathResolver, ValidateDataPathReportsMissingFileForIncompleteMm8DataPath) {
    TemporaryDataDir dir("OpenEnroth_PathResolver_ut_mm8_incomplete");
    dir.touch("anims/magicdod.vid"); // Enough to detect as MM8, but the rest of the MM8 list is missing.

    std::string missingFile;
    EXPECT_FALSE(validateDataPath(Path(dir.path()), &missingFile));
    EXPECT_EQ(missingFile, "anims/mightdod.vid");
}
