#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Win/Unicode.h"

UNIT_TEST(Unicode, InvalidUtf8) {
    EXPECT_EQ(win::toUtf16("\xc3\x28"), L"\xfffd(");
}

UNIT_TEST(Unicode, Empty) {
    EXPECT_EQ(win::toUtf16(""), L"");
    EXPECT_EQ(win::toUtf8(L""), "");
}
