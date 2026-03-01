#include <cstdlib>
#include <string>
#include <filesystem>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"

UNIT_TEST(FileInputStream, Skip) {
    const char *tmpfile = "tmp_test.txt";
    std::string data(3000, 'a');

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    size_t bytes = in.skip(50);
    EXPECT_EQ(bytes, 50);

    bytes = in.skip(2000);
    EXPECT_EQ(bytes, 2000);

    char buf[1024] = {};
    bytes = in.read(buf, 1024);
    EXPECT_EQ(bytes, 950);
    EXPECT_EQ(std::string_view(buf, 950), std::string(950, 'a'));
    in.close();
}

UNIT_TEST(FileInputStream, ExceptionMessages) {
    const char *fileName = "afjhrbluxnkskghelxrigjmgdhckeog.txt";

    EXPECT_FALSE(std::filesystem::exists(fileName));
    EXPECT_THROW_MESSAGE(FileInputStream in(fileName), fileName);
}

UNIT_TEST(FileInputStream, ReadUntil) {
    const char *tmpfile = "tmp_readuntil_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    const char data[] = "hello\0world\0!";
    out.write(data, sizeof(data) - 1);
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readUntil('\0'), "hello");
    EXPECT_EQ(in.readUntil('\0'), "world");
    EXPECT_EQ(in.readAll(), "!");
    in.close();
}

UNIT_TEST(FileInputStream, ReadUntilLargeData) {
    const char *tmpfile = "tmp_readuntil_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // Create data larger than the internal buffer (8192 bytes) to test cross-buffer readUntil.
    std::string first(10000, 'a');
    std::string second(5000, 'b');

    FileOutputStream out(tmpfile);
    out.write(first.data(), first.size());
    char delim = '\0';
    out.write(&delim, 1);
    out.write(second.data(), second.size());
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readUntil('\0'), first);
    EXPECT_EQ(in.readAll(), second);
    in.close();
}

UNIT_TEST(FileInputStream, ReadSmallChunks) {
    const char *tmpfile = "tmp_readchunks_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(20000, 'x');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    std::string result;
    char buf[100];
    while (true) {
        size_t bytes = in.read(buf, sizeof(buf));
        if (bytes == 0) break;
        result.append(buf, bytes);
    }
    EXPECT_EQ(result, data);
    in.close();
}

UNIT_TEST(FileInputStream, ReadAllLargeFile) {
    const char *tmpfile = "tmp_readall_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(50000, 'z');
    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), data);
    in.close();
}

UNIT_TEST(FileInputStream, SkipAndRead) {
    const char *tmpfile = "tmp_skipread_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(20000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);

    // Small skip within first buffer.
    size_t skipped = in.skip(50);
    EXPECT_EQ(skipped, 50);

    // Large skip across buffers.
    skipped = in.skip(15000);
    EXPECT_EQ(skipped, 15000);

    // Read remaining.
    std::string remaining = in.readAll();
    EXPECT_EQ(remaining, data.substr(15050));
    in.close();
}

