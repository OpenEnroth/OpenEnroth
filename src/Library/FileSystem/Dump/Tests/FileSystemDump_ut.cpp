#include <memory>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Null/NullFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Dump/FileSystemDump.h"

UNIT_TEST(FileSystemDump, DumpEmpty) {
    NullFileSystem fs;
    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({{"", FILE_DIRECTORY}}));
}

UNIT_TEST(FileSystemDump, DumpLimitAndContents) {
    MemoryFileSystem fs;
    fs.write("1/2/3", Blob::fromString("123"));

    EXPECT_EQ(dumpFileSystem(&fs, 0, 0), std::vector<FileSystemDumpEntry>());
    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS, 0), std::vector<FileSystemDumpEntry>());

    EXPECT_EQ(dumpFileSystem(&fs, 0, 1), std::vector<FileSystemDumpEntry>({{"", FILE_DIRECTORY}}));
    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS, 1), std::vector<FileSystemDumpEntry>({{"", FILE_DIRECTORY}}));

    EXPECT_EQ(dumpFileSystem(&fs, 0), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"1", FILE_DIRECTORY},
        {"1/2", FILE_DIRECTORY},
        {"1/2/3", FILE_REGULAR, 3},
    }));

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"1", FILE_DIRECTORY},
        {"1/2", FILE_DIRECTORY},
        {"1/2/3", FILE_REGULAR, "123"},
    }));
}
