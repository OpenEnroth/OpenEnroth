#include <cstddef>
#include <algorithm>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/InputStream.h"
#include "Utility/Streams/MemoryInputStream.h"

/**
 * An input stream that doesn't report its size, feeding data through `_underflow` in small chunks.
 * Used for testing the unsized `readAll` code path.
 */
class UnsizedInputStream : public InputStream {
 public:
    UnsizedInputStream(const void *data, size_t size, size_t chunkSize = 128) :
        _data(static_cast<const char *>(data)), _dataSize(size), _chunkSize(chunkSize) {
        open({}, static_cast<size_t>(-1), {});
    }

 protected:
    virtual size_t _underflow(void *data, size_t size, Buffer *buffer) override {
        size_t bytesAvailable = _dataSize - _pos;
        if (bytesAvailable == 0) {
            buffer->reset(nullptr, nullptr, nullptr);
            return 0;
        }

        if (data) {
            // Read path.
            size_t bytesRead = std::min(size, bytesAvailable);
            memcpy(data, _data + _pos, bytesRead);
            _pos += bytesRead;
            buffer->reset(nullptr, nullptr, nullptr);
            return bytesRead;
        } else if (size > 0) {
            // Skip path.
            size_t bytesSkipped = std::min(size, bytesAvailable);
            _pos += bytesSkipped;
            buffer->reset(nullptr, nullptr, nullptr);
            return bytesSkipped;
        } else {
            // Refill path.
            size_t bufSize = std::min(bytesAvailable, _chunkSize);
            const char *start = _data + _pos;
            buffer->reset(start, start, start + bufSize);
            _pos += bufSize;
            return 0;
        }
    }

 private:
    const char *_data;
    size_t _dataSize;
    size_t _chunkSize;
    size_t _pos = 0;
};

UNIT_TEST(InputStream, ReadAll) {
    std::string largeString(10000, 'a');
    MemoryInputStream input(largeString.data(), largeString.size());

    std::string resultingString = input.readAll();
    EXPECT_EQ(largeString, resultingString); // There was a bug in readAll resulting in failures on large files.
}

UNIT_TEST(InputStream, ReadAllToString) {
    MemoryInputStream input("hello world", 11);
    std::string result;
    size_t bytes = input.readAll(&result);
    EXPECT_EQ(bytes, 11);
    EXPECT_EQ(result, "hello world");
}

UNIT_TEST(InputStream, ReadAllToStringClears) {
    MemoryInputStream input("world", 5);
    std::string result = "hello ";
    size_t bytes = input.readAll(&result);
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(result, "world");
}

UNIT_TEST(InputStream, ReadAllEmpty) {
    MemoryInputStream input("", 0);
    std::string result;
    size_t bytes = input.readAll(&result);
    EXPECT_EQ(bytes, 0);
    EXPECT_EQ(result, "");
}

UNIT_TEST(InputStream, ReadOrFailThrowsOnShortRead) {
    MemoryInputStream input("hi", 2);
    char buf[10];
    EXPECT_THROW_MESSAGE(input.readOrFail(buf, 10), "10");
}

UNIT_TEST(InputStream, ReadOrFailErrorIncludesDisplayPath) {
    MemoryInputStream input("hi", 2, "test_stream.bin");
    char buf[10];
    EXPECT_THROW_MESSAGE(input.readOrFail(buf, 10), "test_stream.bin");
}

UNIT_TEST(InputStream, ReadOrFailSucceeds) {
    MemoryInputStream input("hello", 5);
    char buf[5];
    EXPECT_NO_THROW(input.readOrFail(buf, 5));
    EXPECT_EQ(std::string_view(buf, 5), "hello");
}

UNIT_TEST(InputStream, SkipOrFailThrowsOnShortSkip) {
    MemoryInputStream input("hi", 2);
    EXPECT_THROW_MESSAGE(input.skipOrFail(10), "10");
}

UNIT_TEST(InputStream, SkipOrFailErrorIncludesDisplayPath) {
    MemoryInputStream input("hi", 2, "test_stream.bin");
    EXPECT_THROW_MESSAGE(input.skipOrFail(10), "test_stream.bin");
}

UNIT_TEST(InputStream, SkipOrFailSucceeds) {
    MemoryInputStream input("hello", 5);
    EXPECT_NO_THROW(input.skipOrFail(5));
    char buf;
    EXPECT_EQ(input.read(&buf, 1), 0u); // Stream exhausted.
}

UNIT_TEST(InputStream, ReadZeroBytes) {
    MemoryInputStream input("hello", 5);
    EXPECT_EQ(input.read(nullptr, 0), 0u);
    EXPECT_EQ(input.readAll(), "hello"); // Nothing consumed.
}

UNIT_TEST(InputStream, SkipZeroBytes) {
    MemoryInputStream input("hello", 5);
    EXPECT_EQ(input.skip(0), 0u);
    EXPECT_EQ(input.readAll(), "hello"); // Nothing consumed.
}

UNIT_TEST(InputStream, CloseIdempotent) {
    MemoryInputStream input("hello", 5);
    input.close();
    EXPECT_FALSE(input.isOpen());
    EXPECT_NO_THROW(input.close()); // Double close is fine.
    EXPECT_FALSE(input.isOpen());
}

UNIT_TEST(InputStream, ReopenAfterClose) {
    MemoryInputStream input("hello", 5);
    EXPECT_EQ(input.readAll(), "hello");
    input.close();

    input.open("world", 5);
    EXPECT_TRUE(input.isOpen());
    EXPECT_EQ(input.readAll(), "world");
}

UNIT_TEST(InputStream, ReadUntilDelimiterNotFound) {
    MemoryInputStream input("hello", 5);
    std::string result = input.readUntil('\0');
    EXPECT_EQ(result, "hello");
}

UNIT_TEST(InputStream, ReadUntilConsumesDelimiter) {
    MemoryInputStream input("hello\0world", 11);
    std::string first = input.readUntil('\0');
    std::string second = input.readAll();
    EXPECT_EQ(first, "hello");
    EXPECT_EQ(second, "world");
}

UNIT_TEST(InputStream, ReadUntilEmptyBeforeDelimiter) {
    MemoryInputStream input("\0hello", 6);
    std::string result = input.readUntil('\0');
    EXPECT_EQ(result, "");
    EXPECT_EQ(input.readAll(), "hello");
}

UNIT_TEST(InputStream, ReadUntilClearsString) {
    MemoryInputStream input("world\0!", 7);
    std::string result = "hello ";
    size_t bytes = input.readUntil('\0', &result);
    EXPECT_EQ(bytes, 6);
    EXPECT_EQ(result, "world");
}

UNIT_TEST(InputStream, ReadUntilNewlineDelimiter) {
    MemoryInputStream input("line1\nline2\nline3", 17);
    EXPECT_EQ(input.readUntil('\n'), "line1");
    EXPECT_EQ(input.readUntil('\n'), "line2");
    EXPECT_EQ(input.readUntil('\n'), "line3");
}

UNIT_TEST(InputStream, ReadUntilEmpty) {
    MemoryInputStream input("", 0);
    std::string result = input.readUntil('\0');
    EXPECT_EQ(result, "");
}

UNIT_TEST(InputStream, SizeKnownForMemoryStream) {
    MemoryInputStream input("hello", 5);
    EXPECT_EQ(input.size(), 5u);
}

UNIT_TEST(InputStream, SizeUnknownByDefault) {
    InputStream::Buffer buffer;
    // Default InputStream has size_t(-1) as size.
    // We can only test this through MemoryInputStream which sets size.
    MemoryInputStream input("", 0);
    EXPECT_EQ(input.size(), 0u);
}

UNIT_TEST(InputStream, PositionStartsAtZero) {
    MemoryInputStream input("hello world", 11);
    EXPECT_EQ(input.position(), 0u);
}

UNIT_TEST(InputStream, PositionAdvancesOnSkip) {
    MemoryInputStream input("hello world", 11);
    (void) input.skip(3);
    EXPECT_EQ(input.position(), 3u);
}

UNIT_TEST(InputStream, PositionAtEnd) {
    MemoryInputStream input("hello", 5);
    EXPECT_EQ(input.readAll(), "hello");
    EXPECT_EQ(input.position(), 5u);
    EXPECT_EQ(input.position(), input.size());
}

UNIT_TEST(InputStream, PositionAfterPartialRead) {
    MemoryInputStream input("hello world", 11);
    char buf[5];
    input.readOrFail(buf, 5);
    EXPECT_EQ(input.position(), 5u);
    input.readOrFail(buf, 3);
    EXPECT_EQ(input.position(), 8u);
}

UNIT_TEST(InputStream, PositionResetsOnReopen) {
    MemoryInputStream input("hello", 5);
    (void) input.skip(3);
    EXPECT_EQ(input.position(), 3u);

    input.open("world!", 6);
    EXPECT_EQ(input.position(), 0u);
    EXPECT_EQ(input.size(), 6u);
}

UNIT_TEST(InputStream, UnsizedReadAll) {
    std::string data = "hello world";
    UnsizedInputStream input(data.data(), data.size());
    EXPECT_EQ(input.readAll(), "hello world");
}

UNIT_TEST(InputStream, UnsizedReadAllLarge) {
    std::string data(10000, 'x');
    UnsizedInputStream input(data.data(), data.size(), 128);
    EXPECT_EQ(input.readAll(), data);
}

UNIT_TEST(InputStream, UnsizedReadAllEmpty) {
    UnsizedInputStream input("", 0);
    std::string result;
    size_t bytes = input.readAll(&result);
    EXPECT_EQ(bytes, 0u);
    EXPECT_EQ(result, "");
}

