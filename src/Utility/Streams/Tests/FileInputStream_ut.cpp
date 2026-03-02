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

    size_t skipped = in.skip(50);
    EXPECT_EQ(skipped, 50);

    skipped = in.skip(15000);
    EXPECT_EQ(skipped, 15000);

    std::string remaining = in.readAll();
    EXPECT_EQ(remaining, data.substr(15050));
    in.close();
}

UNIT_TEST(FileInputStream, LargeSkip) {
    const char *tmpfile = "tmp_largeskip_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    std::string data(2000, 'a');
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<char>('a' + (i % 26));

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    // Use a small buffer so that skip(200) exceeds buffer size and triggers the seek path.
    FileInputStream in(tmpfile, 128);

    // Small skip: goes through buffered fread.
    size_t skipped = in.skip(50);
    EXPECT_EQ(skipped, 50);

    // Large skip: exceeds buffer size, triggers fseeko.
    skipped = in.skip(200);
    EXPECT_EQ(skipped, 200);

    std::string remaining = in.readAll();
    EXPECT_EQ(remaining, data.substr(250));
    in.close();
}

UNIT_TEST(FileInputStream, LargeSkipPastEnd) {
    const char *tmpfile = "tmp_largeskip_end_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile, 4);

    // Skip more than the file contains.
    size_t skipped = in.skip(1000);
    EXPECT_EQ(skipped, 5);

    EXPECT_EQ(in.readAll(), "");
    in.close();
}

UNIT_TEST(FileInputStream, ReadOrFailThrows) {
    const char *tmpfile = "tmp_readorfail_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hi");
    out.close();

    FileInputStream in(tmpfile);
    char buf[10];
    EXPECT_THROW_MESSAGE(in.readOrFail(buf, 10), "10");
}

UNIT_TEST(FileInputStream, SkipOrFailThrows) {
    const char *tmpfile = "tmp_skiporfail_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hi");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_THROW_MESSAGE(in.skipOrFail(10), "10");
}

UNIT_TEST(FileInputStream, CloseIdempotent) {
    const char *tmpfile = "tmp_closeidem_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    in.close();
    EXPECT_FALSE(in.isOpen());
    EXPECT_NO_THROW(in.close()); // Double close is fine.
    EXPECT_FALSE(in.isOpen());
}

UNIT_TEST(FileInputStream, ReopenAfterClose) {
    const char *tmpfile1 = "tmp_reopen1_test.txt";
    const char *tmpfile2 = "tmp_reopen2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    {
        FileOutputStream out(tmpfile1);
        out.write("first");
        out.close();
    }
    {
        FileOutputStream out(tmpfile2);
        out.write("second");
        out.close();
    }

    FileInputStream in(tmpfile1);
    EXPECT_EQ(in.readAll(), "first");
    in.close();

    in.open(tmpfile2);
    EXPECT_TRUE(in.isOpen());
    EXPECT_EQ(in.readAll(), "second");
    in.close();
}

UNIT_TEST(FileInputStream, ReadUntilMultiRefill) {
    const char *tmpfile = "tmp_readuntil_multirefill_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // 500 bytes of 'a', then a '\0' delimiter, then 200 bytes of 'b'.
    // With a 64-byte buffer, readUntil needs ~8 refills to find the delimiter.
    std::string first(500, 'a');
    std::string second(200, 'b');

    FileOutputStream out(tmpfile);
    out.write(first.data(), first.size());
    char delim = '\0';
    out.write(&delim, 1);
    out.write(second.data(), second.size());
    out.close();

    FileInputStream in(tmpfile, 64);
    EXPECT_EQ(in.readUntil('\0'), first);
    EXPECT_EQ(in.readAll(), second);
    in.close();
}

UNIT_TEST(FileInputStream, ReadUntilNotFound) {
    const char *tmpfile = "tmp_readuntil_notfound_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    // No delimiter in data — readUntil should return everything.
    std::string data(300, 'x');

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile, 64);
    EXPECT_EQ(in.readUntil('\0'), data);
    EXPECT_EQ(in.readAll(), "");
    in.close();
}

UNIT_TEST(FileInputStream, ReadAllEmpty) {
    const char *tmpfile = "tmp_readall_empty_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.close(); // Empty file.

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "");
    in.close();
}

UNIT_TEST(FileInputStream, ReadAllMaxSizeZero) {
    const char *tmpfile = "tmp_readall_maxsize0_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(size_t{0}), "");
    EXPECT_EQ(in.readAll(), "hello"); // Nothing consumed.
    in.close();
}

UNIT_TEST(FileInputStream, ReadZeroBytes) {
    const char *tmpfile = "tmp_readzero_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.read(nullptr, 0), 0u);
    EXPECT_EQ(in.readAll(), "hello"); // Nothing consumed.
}
