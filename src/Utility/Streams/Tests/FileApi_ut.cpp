#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileApi.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Exception.h"

// These tests drive the libc failure paths in the file streams, which are otherwise only reachable when the OS itself
// misbehaves. The injected state is global because the table is - tests run one at a time, so that's enough.

namespace {

struct Injection {
    int writeLimit = -1;         // Bytes `writeBytes` takes before reporting a short write, -1 to pass through.
    bool failRead = false;       // Whether `readBytes` reports an error instead of reading.
    bool failClose = false;
    bool failFlush = false;
    bool failSetBuffering = false;
    int failSeekOnCall = -1;     // 1-based index of the `seek` call to fail, -1 to pass through.
    int failTellOnCall = -1;     // Same for `tell`.
    int seekCalls = 0;
    int tellCalls = 0;
};

Injection injection;

const detail::FileApi *native() {
    return detail::nativeFileApi();
}

size_t writeBytes(const void *data, size_t size, size_t count, FILE *file) {
    if (injection.writeLimit < 0)
        return native()->writeBytes(data, size, count, file);

    // Let the leading bytes through, so the file really does end up holding a prefix of what was asked for.
    size_t accepted = std::min<size_t>(count, injection.writeLimit);
    if (accepted > 0)
        (void) native()->writeBytes(data, size, accepted, file);
    errno = ENOSPC;
    return accepted;
}

size_t readBytes(void *data, size_t size, size_t count, FILE *file) {
    if (!injection.failRead)
        return native()->readBytes(data, size, count, file);
    errno = EIO;
    return 0;
}

int checkError(FILE *file) {
    return injection.failRead ? 1 : native()->checkError(file);
}

int closeFile(FILE *file) {
    int result = native()->closeFile(file);
    if (!injection.failClose)
        return result;
    errno = EIO;
    return EOF;
}

int flush(FILE *file) {
    if (!injection.failFlush)
        return native()->flush(file);
    errno = EIO;
    return EOF;
}

int setBuffering(FILE *file, char *buffer, int mode, size_t size) {
    if (!injection.failSetBuffering)
        return native()->setBuffering(file, buffer, mode, size);
    errno = EINVAL;
    return -1;
}

int seek(FILE *file, int64_t offset, int origin) {
    if (++injection.seekCalls == injection.failSeekOnCall) {
        errno = EINVAL;
        return -1;
    }
    return native()->seek(file, offset, origin);
}

int64_t tell(FILE *file) {
    if (++injection.tellCalls == injection.failTellOnCall) {
        errno = EINVAL;
        return -1;
    }
    return native()->tell(file);
}

detail::FileApi makeApi() {
    injection = Injection();
    detail::FileApi result = *native();
    result.writeBytes = &writeBytes;
    result.readBytes = &readBytes;
    result.checkError = &checkError;
    result.closeFile = &closeFile;
    result.flush = &flush;
    result.setBuffering = &setBuffering;
    result.seek = &seek;
    result.tell = &tell;
    return result;
}

void writeFile(const char *path, const std::string &data) {
    FileOutputStream out(path);
    out.write(data);
    out.close();
}

} // namespace

UNIT_TEST(FileApi, PositionAfterShortWrite) {
    // A write that fails partway still leaves its leading bytes in the file, so `position()` has to move by exactly
    // that much. Reporting no progress here used to desync the stream from the file for good.
    const char *tmpfile = "tmp_short_write.txt";
    ScopedTestFileSlot tmp(tmpfile);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileOutputStream out(tmpfile, 16);
    injection.writeLimit = 40;
    EXPECT_THROW(out.write(std::string(100, 'a')), Exception); // 100 >= 16, so this goes out directly.
    EXPECT_EQ(out.position(), 40u);

    injection.writeLimit = -1;
    out.close();
}

UNIT_TEST(FileApi, PartialSkipIsNotAnError) {
    // Once the file offset has moved, the skip has to report how far it got rather than throw - throwing would strand
    // the offset past what `position()` says, and every later read would come back from the wrong place.
    const char *tmpfile = "tmp_partial_skip.txt";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, std::string(100, 'a'));

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile, 16);
    injection.failRead = true;
    EXPECT_EQ(in.skip(1000), 100u); // Seeks over the 100 bytes the file has, then the read-and-discard tail fails.
    EXPECT_EQ(in.position(), 100u);

    injection.failRead = false;
    in.close();
}

UNIT_TEST(FileApi, SkipReportsErrorWhenNothingWasSkipped) {
    // Nothing to seek over here, so the failing read is the whole operation and has to surface.
    const char *tmpfile = "tmp_skip_error.txt";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, std::string(100, 'a'));

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile, 16);
    EXPECT_EQ(in.skip(100), 100u); // At end of file now.
    injection.failRead = true;
    EXPECT_THROW((void) in.skip(1000), Exception);

    injection.failRead = false;
    in.close();
}

UNIT_TEST(FileApi, ReadErrorIsReported) {
    const char *tmpfile = "tmp_read_error.txt";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, std::string(100, 'a'));

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile, 16);
    injection.failRead = true;
    char buf[8];
    EXPECT_THROW_MESSAGE((void) in.read(buf, sizeof(buf)), tmpfile); // Small read, fills the internal buffer.

    std::string large(1000, 'x');
    EXPECT_THROW_MESSAGE((void) in.read(large.data(), large.size()), tmpfile); // Large read, straight through.

    injection.failRead = false;
    in.close();
}

UNIT_TEST(FileApi, CloseErrorIsReported) {
    const char *tmpfile = "tmp_close_error.txt";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello");

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile);
    injection.failClose = true;
    EXPECT_THROW_MESSAGE(in.close(), tmpfile);
    EXPECT_FALSE(in.isOpen()); // Closed either way, the error doesn't leave it half open.

    FileOutputStream out(tmpfile);
    out.write("hello");
    EXPECT_THROW_MESSAGE(out.close(), tmpfile);
    EXPECT_FALSE(out.isOpen());

    injection.failClose = false;
}

UNIT_TEST(FileApi, FlushErrorIsReported) {
    const char *tmpfile = "tmp_flush_error.txt";
    ScopedTestFileSlot tmp(tmpfile);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileOutputStream out(tmpfile);
    out.write("hello");
    injection.failFlush = true;
    EXPECT_THROW_MESSAGE(out.flush(), tmpfile);
    EXPECT_EQ(out.position(), 5u); // Flushing consumes nothing, so it must not have moved.

    injection.failFlush = false;
    out.close();
}

UNIT_TEST(FileApi, OpenReportsSetupFailures) {
    const char *tmpfile = "tmp_open_failure.txt";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, "hello");

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    // `open` disables libc buffering, then measures the file by seeking to the end and back. A failure in any of
    // those has to come out as an exception rather than as a half-built stream.
    // Opened through `open` rather than the constructor, so that these read as expressions and not as declarations.
    FileInputStream in;
    FileOutputStream out;

    injection.failSetBuffering = true;
    EXPECT_THROW_MESSAGE(in.open(tmpfile), tmpfile);
    EXPECT_THROW_MESSAGE(out.open(tmpfile), tmpfile);
    injection.failSetBuffering = false;

    injection.seekCalls = 0;
    injection.failSeekOnCall = 1; // Seek to the end.
    EXPECT_THROW_MESSAGE(in.open(tmpfile), tmpfile);
    injection.seekCalls = 0;
    injection.failSeekOnCall = 2; // Seek back to the start.
    EXPECT_THROW_MESSAGE(in.open(tmpfile), tmpfile);
    injection.failSeekOnCall = -1;

    injection.tellCalls = 0;
    injection.failTellOnCall = 1;
    EXPECT_THROW_MESSAGE(in.open(tmpfile), tmpfile);
    injection.failTellOnCall = -1;

    EXPECT_FALSE(in.isOpen()); // None of those left a stream behind.
    EXPECT_FALSE(out.isOpen());
}

UNIT_TEST(FileApi, LargeSkipReportsSeekFailures) {
    const char *tmpfile = "tmp_skip_seek_error.txt";
    ScopedTestFileSlot tmp(tmpfile);
    writeFile(tmpfile, std::string(100, 'a'));

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    // A large skip works out where the file ends before moving anything, so it tells, seeks to the end, tells again,
    // then seeks to where it wanted to land. Each of those has its own error path.
    for (int failingSeek : {1, 2}) {
        FileInputStream in(tmpfile, 16);
        injection.seekCalls = 0;
        injection.failSeekOnCall = failingSeek;
        EXPECT_THROW_MESSAGE((void) in.skip(1000), tmpfile);
        injection.failSeekOnCall = -1;
        in.close();
    }

    for (int failingTell : {1, 2}) {
        FileInputStream in(tmpfile, 16);
        injection.tellCalls = 0;
        injection.failTellOnCall = failingTell;
        EXPECT_THROW_MESSAGE((void) in.skip(1000), tmpfile);
        injection.failTellOnCall = -1;
        in.close();
    }
}
