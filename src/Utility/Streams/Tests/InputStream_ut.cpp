#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/MemoryInputStream.h"

// readAll

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

UNIT_TEST(InputStream, ReadAllToStringMaxSize) {
    MemoryInputStream input("hello world", 11);
    std::string result;
    size_t bytes = input.readAll(&result, 5);
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(result, "hello");
}

UNIT_TEST(InputStream, ReadAllEmpty) {
    MemoryInputStream input("", 0);
    std::string result;
    size_t bytes = input.readAll(&result);
    EXPECT_EQ(bytes, 0);
    EXPECT_EQ(result, "");
}

// readUntil on MemoryInputStream

UNIT_TEST(InputStream, ReadUntilDelimiterFound) {
    MemoryInputStream input("hello\0world", 11);
    std::string result = input.readUntil('\0');
    EXPECT_EQ(result, "hello");
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

UNIT_TEST(InputStream, ReadUntilMaxSize) {
    MemoryInputStream input("hello\0world", 11);
    std::string result = input.readUntil('\0', 3);
    EXPECT_EQ(result, "hel");
}

UNIT_TEST(InputStream, ReadUntilClearsString) {
    MemoryInputStream input("world\0!", 7);
    std::string result = "hello ";
    size_t bytes = input.readUntil('\0', &result);
    EXPECT_EQ(bytes, 6);
    EXPECT_EQ(result, "world");
}

UNIT_TEST(InputStream, ReadUntilMultipleStrings) {
    const char data[] = "aaa\0bbb\0ccc";
    MemoryInputStream input(data, sizeof(data) - 1);
    EXPECT_EQ(input.readUntil('\0'), "aaa");
    EXPECT_EQ(input.readUntil('\0'), "bbb");
    EXPECT_EQ(input.readUntil('\0'), "ccc");
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
