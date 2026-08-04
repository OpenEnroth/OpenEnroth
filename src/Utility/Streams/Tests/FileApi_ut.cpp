#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <memory>
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
    int readLimit = -1;          // Bytes `readBytes` returns before reporting an error, -1 to pass through.
    bool failRead = false;       // Whether `readBytes` reports an error instead of reading.
    bool failClose = false;
    bool failFlush = false;
    bool failSetBuffering = false;
    int failSeekOnCall = -1;     // 1-based index of the `seek` call to fail, -1 to pass through.
    int failTellOnCall = -1;     // Same for `tell`.
    int seekCalls = 0;
    int tellCalls = 0;
    int filesOpened = 0;
    int filesClosed = 0;
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
    if (injection.failRead) {
        errno = EIO;
        return 0;
    }
    if (injection.readLimit < 0)
        return native()->readBytes(data, size, count, file);

    size_t accepted = native()->readBytes(data, size, std::min<size_t>(count, injection.readLimit), file);
    errno = EIO;
    return accepted;
}

int checkError(FILE *file) {
    if (injection.failRead || injection.readLimit >= 0)
        return 1;
    return native()->checkError(file);
}

FILE *openFile(const char *path, const char *mode) {
    FILE *result = native()->openFile(path, mode);
    if (result)
        injection.filesOpened++;
    return result;
}

int closeFile(FILE *file) {
    injection.filesClosed++;
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
    result.openFile = &openFile;
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

std::string patternedData(size_t size) {
    // Distinct bytes, so that a misplaced, dropped or duplicated range shows up as a mismatch. A run of identical
    // bytes can't catch any of that.
    std::string result(size, '\0');
    for (size_t i = 0; i < size; i++)
        result[i] = 'a' + i % 26;
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
    EXPECT_GT(injection.filesOpened, 0); // And none of them leaked its `FILE *`.
    EXPECT_EQ(injection.filesClosed, injection.filesOpened);
}

UNIT_TEST(FileApi, LargeSkipReportsSeekFailures) {
    const char *tmpfile = "tmp_skip_seek_error.txt";
    ScopedTestFileSlot tmp(tmpfile);
    std::string data = patternedData(100);
    writeFile(tmpfile, data);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    // A large skip works out where the file ends before moving anything, so it tells, seeks to the end, tells again,
    // then seeks to where it wanted to land. Each of those has its own error path, and each has to leave the file
    // offset where `position()` says. The measuring seek used to strand the offset at the end when a later call
    // failed - the skip threw, and a caller that caught the exception then read EOF where its data was.
    for (int failingSeek : {1, 2}) {
        FileInputStream in(tmpfile, 16);
        injection.seekCalls = 0;
        injection.failSeekOnCall = failingSeek;
        EXPECT_THROW_MESSAGE((void) in.skip(1000), tmpfile);
        injection.failSeekOnCall = -1;
        EXPECT_EQ(in.readAll(), data); // Nothing was skipped, so everything is still there.
        in.close();
    }

    for (int failingTell : {1, 2}) {
        FileInputStream in(tmpfile, 16);
        injection.tellCalls = 0;
        injection.failTellOnCall = failingTell;
        EXPECT_THROW_MESSAGE((void) in.skip(1000), tmpfile);
        injection.failTellOnCall = -1;
        EXPECT_EQ(in.readAll(), data);
        in.close();
    }
}

UNIT_TEST(FileApi, PartialReadWithErrorReturnsTheData) {
    // The read paths throw only when a read returned nothing at all. A read that got some bytes before failing has
    // to come back as data - throwing would lose the bytes it got - and the error resurfaces on the next call.
    const char *tmpfile = "tmp_partial_read.txt";
    ScopedTestFileSlot tmp(tmpfile);
    std::string data = patternedData(100);
    writeFile(tmpfile, data);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile, 16);
    char buf[50];
    injection.readLimit = 20;
    EXPECT_EQ(in.read(buf, sizeof(buf)), 20u); // Large read, straight through.
    EXPECT_EQ(std::string(buf, 20), data.substr(0, 20));
    injection.readLimit = -1;
    injection.failRead = true;
    EXPECT_THROW((void) in.read(buf, sizeof(buf)), Exception); // Nothing came back this time, so it surfaces.
    injection.failRead = false;
    EXPECT_EQ(in.readAll(), data.substr(20));
    in.close();

    FileInputStream small(tmpfile, 16);
    injection.readLimit = 4;
    EXPECT_EQ(small.read(buf, 8), 4u); // Small read, via the internal buffer.
    EXPECT_EQ(std::string(buf, 4), data.substr(0, 4));
    injection.readLimit = -1;
    small.close();
}

UNIT_TEST(FileApi, WriteResumesAfterShortWrite) {
    // The point of the position accounting - after a failed write, `position()` is exactly where the next byte lands,
    // so the caller can retry the unaccepted tail and end up with the file it meant to write.
    const char *tmpfile = "tmp_write_resume.txt";
    ScopedTestFileSlot tmp(tmpfile);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    std::string data = patternedData(100);
    FileOutputStream out(tmpfile, 16);
    injection.writeLimit = 40;
    EXPECT_THROW(out.write(data), Exception);
    injection.writeLimit = -1;

    out.write(data.substr(out.position()));
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), data);
    in.close();
}

UNIT_TEST(FileApi, CloseDoesNotDuplicateAfterFailedFlush) {
    // A flush that fails partway has still pushed a prefix of the buffer out. Those bytes are dropped from the
    // buffer, and used to not be - `close` then wrote them to the file a second time.
    const char *tmpfile = "tmp_flush_resume.txt";
    ScopedTestFileSlot tmp(tmpfile);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    std::string data = patternedData(50);
    FileOutputStream out(tmpfile, 64);
    out.write(data);
    injection.writeLimit = 20;
    EXPECT_THROW(out.flush(), Exception);
    EXPECT_EQ(out.position(), 50u); // Flushing consumes nothing, whether or not it fails.
    injection.writeLimit = -1;
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), data);
    in.close();
}

UNIT_TEST(FileApi, ReadResumesAfterError) {
    // A read that fails consumes nothing past what it drained from the buffer, and `position()` says how much that
    // was. Once the error clears, reading picks up from exactly there.
    const char *tmpfile = "tmp_read_resume.txt";
    ScopedTestFileSlot tmp(tmpfile);
    std::string data = patternedData(100);
    writeFile(tmpfile, data);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile, 16);
    char buf[50];
    in.readOrFail(buf, 10); // Leaves 6 buffered bytes, which the failing read below drains first.
    injection.failRead = true;
    EXPECT_THROW((void) in.read(buf, sizeof(buf)), Exception);
    injection.failRead = false;

    size_t pos = in.position();
    EXPECT_EQ(pos, 16u); // The 10 bytes read, plus the 6 the failing read drained from the buffer before dying.
    EXPECT_EQ(in.readAll(), data.substr(pos));
    in.close();
}

UNIT_TEST(FileApi, ReopenAfterFailedClose) {
    // A close that fails still tears the stream down, so the same object can be opened again.
    const char *tmpfile = "tmp_reopen_after_close.txt";
    ScopedTestFileSlot tmp(tmpfile);
    std::string data = patternedData(30);
    writeFile(tmpfile, data);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileInputStream in(tmpfile);
    injection.failClose = true;
    EXPECT_THROW(in.close(), Exception);
    injection.failClose = false;
    in.open(tmpfile);
    EXPECT_EQ(in.readAll(), data);
    in.close();

    FileOutputStream out(tmpfile);
    out.write("garbage");
    injection.failClose = true;
    EXPECT_THROW(out.close(), Exception);
    injection.failClose = false;
    out.open(tmpfile);
    out.write(data);
    out.close();

    FileInputStream check(tmpfile);
    EXPECT_EQ(check.readAll(), data);
    check.close();
}

UNIT_TEST(FileApi, WriteErrorWinsOverCloseError) {
    // When both the final flush and the close itself fail, the write error is the one that says what was lost, so
    // it's the one that has to surface. ENOSPC mentions space on all platforms, EIO doesn't.
    const char *tmpfile = "tmp_error_precedence.txt";
    ScopedTestFileSlot tmp(tmpfile);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    FileOutputStream out(tmpfile);
    out.write("hello");
    injection.writeLimit = 0;
    injection.failClose = true;
    EXPECT_THROW_MESSAGE(out.close(), "space");
    EXPECT_FALSE(out.isOpen()); // Closed either way.
    injection.writeLimit = -1;
    injection.failClose = false;
}

UNIT_TEST(FileApi, DestructorSwallowsBothErrors) {
    // The destructor closes with throwing disabled, so even a failing flush plus a failing close have to come out as
    // nothing - throwing there would terminate the process.
    const char *tmpfile = "tmp_dtor_both_errors.txt";
    ScopedTestFileSlot tmp(tmpfile);

    detail::FileApi api = makeApi();
    detail::ScopedFileApi scope(&api);

    auto out = std::make_unique<FileOutputStream>(tmpfile);
    out->write("hello");
    injection.writeLimit = 0;
    injection.failClose = true;
    EXPECT_NO_THROW(out.reset());
    injection.writeLimit = -1;
    injection.failClose = false;
}
