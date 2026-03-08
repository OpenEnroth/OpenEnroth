#include <cstdlib>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"

UNIT_TEST(FileOutputStream, Write) {
    const char *tmpfile = "tmp_test.txt";
    const char *tmpfilecontent = "1234\n";
    size_t tmpfilesize = strlen(tmpfilecontent);

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write(tmpfilecontent, tmpfilesize);
    out.close();

    FileInputStream in(tmpfile);

    char buf[1024] = {};
    size_t bytes = in.read(buf, 1024);;
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(strcmp(buf, tmpfilecontent), 0);

    bytes = in.read(buf, 1024);
    EXPECT_EQ(bytes, 0);
    in.close();
}

UNIT_TEST(FileOutputStream, FlushMidStream) {
    const char *tmpfile = "tmp_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.flush();

    {
        FileInputStream in(tmpfile);
        EXPECT_EQ(in.readAll(), "hello");
    }

    out.write(" world");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello world");
}

UNIT_TEST(FileOutputStream, LargeWriteBypassesBuffer) {
    // Use a small buffer so that a large write goes through the direct-write path in _overflow.
    const char *tmpfile = "tmp_largewrite_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);
    std::string large(1024, 'x');
    out.write(large.data(), large.size());
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), large);
}

UNIT_TEST(FileOutputStream, MixedSmallAndLargeWrites) {
    const char *tmpfile = "tmp_mixed_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);

    std::string expected;

    out.write("hello");
    expected += "hello";

    std::string large(256, 'y');
    out.write(large.data(), large.size());
    expected += large;

    out.write(" end");
    expected += " end";

    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), expected);
}

UNIT_TEST(FileOutputStream, CloseIdempotent) {
    const char *tmpfile = "tmp_closeidem_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.close();
    EXPECT_FALSE(out.isOpen());
    EXPECT_NO_THROW(out.close()); // Double close is fine.
    EXPECT_FALSE(out.isOpen());
}

UNIT_TEST(FileOutputStream, ReopenAfterClose) {
    const char *tmpfile = "tmp_reopen_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("first");
    out.close();

    out.open(tmpfile);
    out.write("second");
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "second"); // File is overwritten, not appended.
}

UNIT_TEST(FileOutputStream, PositionStartsAtZero) {
    const char *tmpfile = "tmp_pos_start_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    EXPECT_EQ(out.position(), 0u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionAdvancesOnWrite) {
    const char *tmpfile = "tmp_pos_write_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    EXPECT_EQ(out.position(), 5u);
    out.write(" world");
    EXPECT_EQ(out.position(), 11u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionAfterFlush) {
    const char *tmpfile = "tmp_pos_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    out.flush();
    EXPECT_EQ(out.position(), 5u);
    out.write(" world");
    EXPECT_EQ(out.position(), 11u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionAfterLargeWrite) {
    const char *tmpfile = "tmp_pos_large_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 64);
    std::string large(1024, 'x');
    out.write(large);
    EXPECT_EQ(out.position(), 1024u);
    out.close();
}

UNIT_TEST(FileOutputStream, PositionResetsOnReopen) {
    const char *tmpfile = "tmp_pos_reopen_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write("hello");
    EXPECT_EQ(out.position(), 5u);
    out.close();

    out.open(tmpfile);
    EXPECT_EQ(out.position(), 0u);
    out.close();
}
