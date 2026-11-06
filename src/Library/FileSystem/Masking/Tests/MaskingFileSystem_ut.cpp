#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Masking/MaskingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(MaskingFileSystem, SimpleMasks) {
    MemoryFileSystem fs0("");
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

    fs1.mask("a/b/c/e");
    EXPECT_FALSE(fs1.exists("a/b/c/e"));
    EXPECT_TRUE(fs0.exists("a/b/c/e"));
    EXPECT_EQ(fs1.ls("a/b/c"), std::vector<DirectoryEntry>()); // Masking might result in observable empty dirs.

    fs1.mask("");
    EXPECT_EQ(fs1.ls(""), std::vector<DirectoryEntry>());

    EXPECT_TRUE(fs1.unmask("a/b/c/e"));
    EXPECT_FALSE(fs1.exists("a/b/c/e")); // Still masked.
    EXPECT_TRUE(fs1.unmask(""));
    EXPECT_TRUE(fs1.exists("a/b/c/e"));
    EXPECT_FALSE(fs1.exists("a/b/c/d"));

    fs1.clearMasks();
    EXPECT_TRUE(fs1.exists("a/b/c/d"));
}

UNIT_TEST(MaskingFileSystem, PersistentMasking) {
    MemoryFileSystem fs0("");
    MaskingFileSystem fs1(&fs0);

    fs1.mask("a");
    fs0.write("a", Blob());
    EXPECT_FALSE(fs1.exists("a"));
    EXPECT_TRUE(fs0.exists("a"));
}
