#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Lod/LodReader.h"

const char brokenLod[] =
    "LOD\0"         "Game"          "MMVI"          "\0\0\0\0"      // signature, version
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "Maps"          " for"          " MMV"          // description
    "I\0\0\0"       "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\x64\0\0\0"    "\0\0\0\0"      "\1\0\0\0"      // size = 100, unk_0 = 0, numDirectories = 1
    "I\0\0\0"       "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      // unk_1
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"
    "maps"          "\0\0\0\0"      "\0\0\0\0"      "\0\0\0\0"      // name
    "\x20\x01\0\0"  "\x21\0\0\0"    "\0\0\0\0"      "\x01\0\0\0"    // dataOffset = 288, dataSize = 33, unk_0 = 0, numItems = 1, priority = 0
                                                                    //                              ^ actual data size is 48
    "lolk"          "ek\0\0"        "\0\0\0\0"      "\0\0\0\0"      // name
    "\x20\0\0\0"    "\x10\0\0\0"    "\0\0\0\0"      "\0\0\0\0"      // dataOffset = 32, dataSize = 16, unk_0 = 0, numItems = 0, priority = 0
    "data"          "data"          "data"          "data";

UNIT_TEST(LodReader, RussianLod) {
    // Opening a LOD with invalid directory dataSize should just work.
    LodReader reader(Blob::view(brokenLod, sizeof(brokenLod)).withDisplayPath("russian.lod"), LOD_ALLOW_DUPLICATES);
    EXPECT_TRUE(reader.isOpen());
    EXPECT_EQ(reader.ls(), std::vector<std::string>{"lolkek"});
    EXPECT_EQ(reader.info().rootName, "maps");
    EXPECT_EQ(reader.info().description, "Maps for MMVI");
    EXPECT_EQ(reader.info().version, LOD_VERSION_MM6_GAME);
    EXPECT_TRUE(reader.exists("lolkek"));
    EXPECT_FALSE(reader.exists("lolkek1"));
    EXPECT_FALSE(reader.exists("lolke"));
    EXPECT_EQ(reader.read("lolkek").string_view(), "datadatadatadata");

    // LODs are case-insensitive.
    EXPECT_TRUE(reader.exists("lolKEK"));
    EXPECT_EQ(reader.read("LOLkek").string_view(), "datadatadatadata");

    // Check that we throw when accessing non-existent files.
    EXPECT_THROW((void) reader.read("lolke"), std::exception);
}

UNIT_TEST(LodReader, ErrorMessage) {
    std::string_view name = "XXXXXXXXXXX";
    Blob blob = Blob().withDisplayPath(name);

    EXPECT_THROW_MESSAGE(LodReader(Blob::share(blob)), name);
}

UNIT_TEST(LodReader, DisplayPath) {
    LodReader reader(Blob::view(brokenLod, sizeof(brokenLod)).withDisplayPath("russian.lod"), LOD_ALLOW_DUPLICATES);
    EXPECT_EQ(reader.read("lolkek").displayPath(), "russian.lod/lolkek");
    EXPECT_EQ(reader.read("LOLKEK").displayPath(), "russian.lod/LOLKEK");
}
