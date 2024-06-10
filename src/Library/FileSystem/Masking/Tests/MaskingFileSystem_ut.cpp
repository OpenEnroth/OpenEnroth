#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Masking/MaskingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(MaskingFileSystem, SimpleMasks) {
    MemoryFileSystem fs0;
    fs0.write("a/b/c/d", Blob());
    fs0.write("a/b/c/e", Blob());
    fs0.write("a/b/1/d", Blob());
    fs0.write("a/b/1/e", Blob());

    MaskingFileSystem fs1(&fs0);
    EXPECT_TRUE(fs1.exists("a"));
    EXPECT_TRUE(fs1.exists("a/b/c/d"));

    fs1.mask("a/b/c/d");
    EXPECT_FALSE(fs1.exists("a/b/c/d"));
    EXPECT_TRUE(fs0.exists("a/b/c/d"));
    EXPECT_EQ(fs1.ls("a/b/c"), std::vector<DirectoryEntry>({{"e", FILE_REGULAR}}));

    fs1.clearMasks();
    EXPECT_TRUE(fs1.exists("a/b/c/d"));

    EXPECT_TRUE(fs1.remove("a/b/c/d"));
    EXPECT_FALSE(fs1.exists("a/b/c/d"));
    EXPECT_TRUE(fs0.exists("a/b/c/d"));
    EXPECT_EQ(fs1.ls("a/b/c"), std::vector<DirectoryEntry>({{"e", FILE_REGULAR}}));

    EXPECT_TRUE(fs1.remove("a/b"));
    EXPECT_TRUE(fs1.exists("a"));
    EXPECT_FALSE(fs1.exists("a/b"));
    EXPECT_FALSE(fs1.exists("a/b/1"));
    EXPECT_FALSE(fs1.exists("a/b/c/e"));
    EXPECT_EQ(fs1.ls("a"), std::vector<DirectoryEntry>());
    EXPECT_TRUE(fs0.exists("a"));
    EXPECT_TRUE(fs0.exists("a/b"));
    EXPECT_TRUE(fs0.exists("a/b/1"));
    EXPECT_TRUE(fs0.exists("a/b/c/e"));
    EXPECT_EQ(fs0.ls("a"), std::vector<DirectoryEntry>({{"b", FILE_DIRECTORY}}));

    EXPECT_FALSE(fs1.remove("a/b/c"));
}

UNIT_TEST(MaskingFileSystem, PersistentMasking) {
    MemoryFileSystem fs0;
    MaskingFileSystem fs1(&fs0);

    fs1.mask("a");
    fs0.write("a", Blob());
    EXPECT_FALSE(fs1.exists("a"));
    EXPECT_TRUE(fs0.exists("a"));
}
