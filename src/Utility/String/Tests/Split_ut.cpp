#include <string>
#include <vector>
#include <set>
#include <unordered_set>

#include <ranges>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Split.h"

static_assert(std::ranges::view<detail::SplitView<detail::CharSplitter>>);
static_assert(std::ranges::viewable_range<detail::SplitView<detail::CharSplitter>>);
static_assert(std::ranges::forward_range<detail::SplitView<detail::CharSplitter>>);
static_assert(std::ranges::view<detail::SplitView<detail::CrLfSplitter>>);
static_assert(std::ranges::viewable_range<detail::SplitView<detail::CrLfSplitter>>);
static_assert(std::ranges::forward_range<detail::SplitView<detail::CrLfSplitter>>);

UNIT_TEST(StringSplit, SplitToVector) {
    std::vector<std::string_view> v;

    split("aa;bb;cc").by(';').to(&v);
    std::vector<std::string_view> r0 = {"aa", "bb", "cc"};
    EXPECT_EQ(v, r0);

    split("ABC").by(';').to(&v);
    std::vector<std::string_view> r1 = {"ABC"};
    EXPECT_EQ(v, r1);

    split("AB").by('B').to(&v);
    std::vector<std::string_view> r2 = {"A", ""};
    EXPECT_EQ(v, r2);

    split(";").by(';').to(&v);
    std::vector<std::string_view> r3 = {"", ""};
    EXPECT_EQ(v, r3);

    split("").by(';').to(&v);
    std::vector<std::string_view> r4 = {""};
    EXPECT_EQ(v, r4);
}

UNIT_TEST(StringSplit, ConverToContainer) {
    const char *s = "1;2;3";

    std::vector<std::string_view> v1 = split(s).by(';');
    EXPECT_EQ(v1, std::vector<std::string_view>({"1", "2", "3"}));

    std::vector<std::string> v2 = split(s).by(';');
    EXPECT_EQ(v2, std::vector<std::string>({"1", "2", "3"}));

    std::set<std::string_view> v3 = split(s).by(';');
    EXPECT_EQ(v3, std::set<std::string_view>({"1", "2", "3"}));

    std::set<std::string> v4 = split(s).by(';');
    EXPECT_EQ(v4, std::set<std::string>({"1", "2", "3"}));

    std::unordered_set<std::string_view> v5 = split(s).by(';');
    EXPECT_EQ(v5, std::unordered_set<std::string_view>({"1", "2", "3"}));

    std::unordered_set<std::string> v6 = split(s).by(';');
    EXPECT_EQ(v6, std::unordered_set<std::string>({"1", "2", "3"}));
}

UNIT_TEST(StringSplit, NonNullTerminated) {
    const char *s = "1,2,3,4,5,6,7,8,9,10";
    std::string_view sv(s, s+9);

    std::vector<std::string_view> v1 = split(sv).by(',');
    EXPECT_EQ(v1, std::vector<std::string_view>({"1", "2", "3", "4", "5"}));
}

UNIT_TEST(StringSplit, DefaultConstructed) {
    detail::SplitView<detail::CharSplitter> view;
    EXPECT_EQ(view.begin(), view.end());
    EXPECT_TRUE(view.empty());

    detail::SplitViewIterator<detail::CharSplitter> end;
    EXPECT_EQ(end, detail::SplitViewSentinel());
    EXPECT_EQ(end, end);

    EXPECT_NO_THROW((
        [] {
            for (std::string_view chunk : detail::SplitView<detail::CharSplitter>())
                throw 0; // Shouldn't get here.
        } ()
    ));
}

UNIT_TEST(StringSplit, ForwardRange) {
    auto ss = split("1.2.3").by('.');

    auto pos = ss.begin();
    EXPECT_EQ(*pos, "1");
    EXPECT_EQ(*pos, "1"); // Double deref is OK.

    auto pos0 = pos;
    auto pos1 = pos++;
    EXPECT_EQ(*pos0, "1"); // Copying works.
    EXPECT_EQ(*pos1, "1"); // Suffix++ works...
    EXPECT_EQ(*pos, "2"); // ...as expected.

    auto pos2 = ++pos;
    EXPECT_EQ(*pos2, "3"); // Prefix++ works as expected too.
    EXPECT_EQ(*pos, "3");

    pos++;
    EXPECT_EQ(pos, ss.end());
}

UNIT_TEST(StringSplit, BorrowedRange) {
    auto mismatch = std::ranges::mismatch(split("a/b/c").by('/'), split("a.b.z").by('.'));
    EXPECT_EQ(*mismatch.in1, "c");
    EXPECT_EQ(*mismatch.in2, "z");
}

UNIT_TEST(StringSplit, CrLf) {
    std::vector<std::string_view> v;

    // Unix line endings (\n only)
    split("line1\nline2\nline3").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "line2", "line3"}));

    // Windows line endings (\r\n)
    split("line1\r\nline2\r\nline3").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "line2", "line3"}));

    // Mixed line endings
    split("line1\nline2\r\nline3").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "line2", "line3"}));

    // Empty string
    split("").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({""}));

    // Single newline
    split("\n").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"", ""}));

    // Single CRLF
    split("\r\n").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"", ""}));

    // Trailing newline
    split("line1\nline2\n").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "line2", ""}));

    // Trailing CRLF
    split("line1\r\nline2\r\n").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "line2", ""}));

    // Multiple consecutive newlines
    split("line1\n\nline2").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "", "line2"}));

    // Multiple consecutive CRLFs
    split("line1\r\n\r\nline2").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1", "", "line2"}));

    // String with no newlines
    split("single line").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"single line"}));

    // Lone \r characters (should not be treated as line ending)
    split("line1\rline2\nline3").byCrLf().to(&v);
    EXPECT_EQ(v, std::vector<std::string_view>({"line1\rline2", "line3"}));
}
