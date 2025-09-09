#include <vector>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Wrap.h"

UNIT_TEST(StringWrap, BasicSentence) {
    std::string text = "This is a simple test";
    size_t width = 10;
    std::vector<std::string> expected = {"This is a", "simple", "test"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, EmptyString) {
    std::string text = "";
    size_t width = 10;
    std::vector<std::string> expected = {""};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, SingleShortWord) {
    std::string text = "Hello";
    size_t width = 10;
    std::vector<std::string> expected = {"Hello"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, SingleLongWord) {
    std::string text = "Supercalifragilisticexpialidocious";
    size_t width = 10;
    std::vector<std::string> expected = {"Supercalif", "ragilistic", "expialidoc", "ious"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, WordsLongerThanWidth) {
    std::string text = "Hello Supercalifragilisticexpialidocious World";
    size_t width = 10;
    std::vector<std::string> expected = {"Hello", "Supercalif", "ragilistic", "expialidoc", "ious World"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, ExactFitWidth) {
    std::string text = "1234567890 12345";
    size_t width = 10;
    std::vector<std::string> expected = {"1234567890", "12345"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, LeadingTrailingSpaces) {
    std::string text = "   Hello world  ";
    size_t width = 10;
    std::vector<std::string> expected = {"   Hello", "world  "};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, WidthGreaterThanTextLength) {
    std::string text = "Hello world";
    size_t width = 20;
    std::vector<std::string> expected = {"Hello world"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, WidthEqualToTextLength) {
    std::string text = "Hello world";
    size_t width = 11;
    std::vector<std::string> expected = {"Hello world"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, WidthOne) {
    std::string text = "AB CD EFGH";
    size_t width = 1;
    std::vector<std::string> expected = {"A", "B", "C", "D", "E", "F", "G", "H"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, AllSpaces) {
    std::string text = "     ";
    size_t width = 10;
    std::vector<std::string> expected = {"     "};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, ForcedLineBreak) {
    std::string text = "Hello\nWorld";
    size_t width = 10;
    std::vector<std::string> expected = {"Hello", "World"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, ForcedLineBreakWrap) {
    std::string text = "This is a\ntest with forced\nline breaks.";
    size_t width = 15;
    std::vector<std::string> expected = {"This is a", "test with", "forced", "line breaks."};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, ForcedLineBreakRepeated) {
    std::string text = "Hello\n\nWorld";
    size_t width = 10;
    std::vector<std::string> expected = {"Hello", "", "World"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, NewlineWithLongWordExceedingWidth) {
    std::string text = "Hello\nSupercalifragilisticexpialidocious\nWorld";
    size_t width = 10;
    std::vector<std::string> expected = {"Hello", "Supercalif", "ragilistic", "expialidoc", "ious", "World"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, NewlineAtEndOfText) {
    std::string text = "This is a test\n";
    size_t width = 15;
    std::vector<std::string> expected = {"This is a test", ""};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, NewlineAtStartOfText) {
    std::string text = "\nThis is a test";
    size_t width = 10;
    std::vector<std::string> expected = {"", "This is a", "test"};
    EXPECT_EQ(wrapText(text, width), expected);
}

UNIT_TEST(StringWrap, MultipleNewlinesOnly) {
    std::string text = "\n\n\n";
    size_t width = 10;
    std::vector<std::string> expected = {"", "", "", ""};
    EXPECT_EQ(wrapText(text, width), expected);
}
