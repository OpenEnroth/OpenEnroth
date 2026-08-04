#include <cstdlib>
#include <memory>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

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

UNIT_TEST(FileOutputStream, ReopenWithoutClose) {
    const char *tmpfile1 = "tmp_outreopen_noclose1_test.txt";
    const char *tmpfile2 = "tmp_outreopen_noclose2_test.txt";
    ScopedTestFileSlot tmp1(tmpfile1);
    ScopedTestFileSlot tmp2(tmpfile2);

    std::string data(2000, 'z');

    FileOutputStream out(tmpfile1, 64);
    out.write("first");

    // Must flush the previous file rather than drop it, and has to reallocate the buffer. This used to be a heap
    // buffer overflow - the old, smaller buffer was kept, and writing into it overran the allocation.
    out.open(tmpfile2, 4096);
    out.write(data);
    out.close();

    FileInputStream in1(tmpfile1);
    EXPECT_EQ(in1.readAll(), "first");

    FileInputStream in2(tmpfile2);
    EXPECT_EQ(in2.readAll(), data);
}

#ifdef __linux__
UNIT_TEST(FileOutputStream, PositionSurvivesFailedFlush) {
    // `/dev/full` can be opened for writing, but every write into it fails with `ENOSPC`.
    FileOutputStream out("/dev/full", 1024);

    out.write(std::string(200, 'y'));
    EXPECT_EQ(out.position(), 200u);

    EXPECT_THROW(out.flush(), Exception);
    EXPECT_EQ(out.position(), 200u); // A failed flush must not move it, in either direction.

    out.write(std::string(10, 'z'));
    EXPECT_EQ(out.position(), 210u);

    // Closing fails too, and that's fine.
    try {
        out.close();
    } catch (const Exception &) {
    }
}
#endif

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

UNIT_TEST(FileOutputStream, DestructorFlushesBuffer) {
    const char *tmpfile = "tmp_dtor_flush_test.txt";
    ScopedTestFileSlot tmp(tmpfile);

    {
        FileOutputStream out(tmpfile);
        out.write("hello");
        // No explicit close() - destructor should flush.
    }

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "hello");
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

UNIT_TEST(FileOutputStream, OpenFailure) {
    // Opening into a directory that doesn't exist has to throw, and the message has to name the path.
    EXPECT_THROW_MESSAGE(FileOutputStream("no_such_dir_here/out.txt"), "no_such_dir_here");
}

UNIT_TEST(FileOutputStream, RepeatedOverflow) {
    // Every write past the buffer end goes through `_overflow`. The second one finds the internal buffer already
    // allocated, which is a different path through it than the first.
    const char *tmpfile = "tmp_repeated_overflow.txt";
    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile, 16);
    for (int i = 0; i < 5; i++)
        out.write(std::string(10, 'a' + i));
    out.close();

    FileInputStream in(tmpfile);
    std::string expected;
    for (int i = 0; i < 5; i++)
        expected += std::string(10, 'a' + i);
    EXPECT_EQ(in.readAll(), expected);
    in.close();
}

#ifdef __linux__
UNIT_TEST(FileOutputStream, DestructorSwallowsWriteError) {
    // Writes to /dev/full fail with ENOSPC. The destructor closes with `canThrow` false, so it has to swallow that -
    // throwing there would terminate the process.
    auto out = std::make_unique<FileOutputStream>("/dev/full");
    out->write(std::string(100, 'x'));
    EXPECT_NO_THROW(out.reset());
}
#endif
