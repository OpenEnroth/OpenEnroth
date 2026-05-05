#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <tuple>

#include "Testing/Unit/UnitTest.h"

#include "Utility/View.h"
#include "Utility/Segment.h"
#include "Utility/String/Split.h"

static_assert(std::ranges::view<detail::ViewWrapper<std::ranges::ref_view<std::vector<int>>>>);
static_assert(std::ranges::random_access_range<detail::ViewWrapper<std::ranges::ref_view<std::vector<int>>>>);

UNIT_TEST(View, Drop) {
    std::vector<int> v = {1, 2, 3, 4, 5};

    std::vector<int> r1 = view(v).drop(2);
    EXPECT_EQ(r1, std::vector<int>({3, 4, 5}));

    std::vector<int> r2 = view(v).drop(0);
    EXPECT_EQ(r2, std::vector<int>({1, 2, 3, 4, 5}));

    std::vector<int> r3 = view(v).drop(5);
    EXPECT_EQ(r3, std::vector<int>({}));

    std::vector<int> r4 = view(v).drop(100);
    EXPECT_EQ(r4, std::vector<int>({}));
}

UNIT_TEST(View, Take) {
    std::vector<int> v = {1, 2, 3, 4, 5};

    std::vector<int> r1 = view(v).take(2);
    EXPECT_EQ(r1, std::vector<int>({1, 2}));

    std::vector<int> r2 = view(v).take(0);
    EXPECT_EQ(r2, std::vector<int>({}));

    std::vector<int> r3 = view(v).take(5);
    EXPECT_EQ(r3, std::vector<int>({1, 2, 3, 4, 5}));

    std::vector<int> r4 = view(v).take(100);
    EXPECT_EQ(r4, std::vector<int>({1, 2, 3, 4, 5}));
}

UNIT_TEST(View, Skip) {
    std::vector<std::string_view> v = {"hello", "", "world", "", "", "!"};

    std::vector<std::string_view> r;
    view(v).skip("").to(&r);
    EXPECT_EQ(r, std::vector<std::string_view>({"hello", "world", "!"}));
}

UNIT_TEST(View, Resize) {
    std::vector<int> v = {1, 2, 3};

    std::vector<int> r1 = view(v).resize(5, 0);
    EXPECT_EQ(r1, std::vector<int>({1, 2, 3, 0, 0}));

    // Already long enough — no padding.
    std::vector<int> r2 = view(v).resize(3, 0);
    EXPECT_EQ(r2, std::vector<int>({1, 2, 3}));

    // Shorter — truncated.
    std::vector<int> r3 = view(v).resize(2, 0);
    EXPECT_EQ(r3, std::vector<int>({1, 2}));

    // Empty range.
    std::vector<int> empty;
    std::vector<int> r4 = view(empty).resize(3, 42);
    EXPECT_EQ(r4, std::vector<int>({42, 42, 42}));

    // Returns reference when inner iterator returns reference (e.g. vector).
    auto resizedRef = view(empty).resize(5, 0);
    static_assert(std::is_reference_v<decltype(*resizedRef.begin())>);

    // Returns by value when inner iterator returns by value (e.g. transform_view).
    auto resizedVal = view(empty).replace(0, 1).resize(5, 0);
    static_assert(!std::is_reference_v<decltype(*resizedVal.begin())>);
}

UNIT_TEST(View, Zip) {
    std::vector<std::string_view> v = {"a", "b", "c"};

    std::vector<std::tuple<std::string_view, int>> r;
    view(v).zip(Segment(1, 3)).to(&r);

    using T = std::tuple<std::string_view, int>;
    std::vector<T> expected = {T{"a", 1}, T{"b", 2}, T{"c", 3}};
    EXPECT_EQ(r, expected);
}

UNIT_TEST(View, To) {
    std::vector<int> v = {1, 2, 3};

    std::vector<int> r;
    view(v).to(&r);
    EXPECT_EQ(r, std::vector<int>({1, 2, 3}));
}

UNIT_TEST(View, ConvertToContainer) {
    std::vector<int> v = {1, 2, 3};

    std::vector<int> r = view(v);
    EXPECT_EQ(r, std::vector<int>({1, 2, 3}));
}

UNIT_TEST(View, AssignToContainer) {
    std::vector<int> v = {1, 2, 3};

    std::vector<int> r;
    r = view(v);
    EXPECT_EQ(r, std::vector<int>({1, 2, 3}));
}

UNIT_TEST(View, Chaining) {
    std::vector<std::string_view> v = {"header", "", "a", "", "b", "c"};

    std::vector<std::string_view> r;
    view(v).drop(1).skip("").to(&r);
    EXPECT_EQ(r, std::vector<std::string_view>({"a", "b", "c"}));
}

UNIT_TEST(View, FullChain) {
    std::vector<std::string_view> v = {"header", "a", "b", "c"};

    std::vector<std::tuple<std::string_view, int>> r;
    view(v).drop(1).zip(Segment(1, 3)).to(&r);

    using T = std::tuple<std::string_view, int>;
    std::vector<T> expected = {T{"a", 1}, T{"b", 2}, T{"c", 3}};
    EXPECT_EQ(r, expected);
}

UNIT_TEST(View, SizeAndEmpty) {
    std::vector<int> v = {1, 2, 3};

    auto wrapped = view(v);
    EXPECT_EQ(wrapped.size(), 3u);
    EXPECT_FALSE(wrapped.empty());

    auto dropped = view(v).drop(3);
    EXPECT_TRUE(dropped.empty());
}

UNIT_TEST(View, Replace) {
    std::vector<std::string_view> v = {"a", "", "b", "", "c"};

    std::vector<std::string_view> r;
    view(v).replace("", "0").to(&r);
    EXPECT_EQ(r, std::vector<std::string_view>({"a", "0", "b", "0", "c"}));
}

UNIT_TEST(View, ReplaceInt) {
    std::vector<int> v = {1, 0, 2, 0, 3};

    std::vector<int> r = view(v).replace(0, -1);
    EXPECT_EQ(r, std::vector<int>({1, -1, 2, -1, 3}));
}

UNIT_TEST(View, ExplicitConversion) {
    // string_view -> string is explicit, should still work via emplace.
    std::vector<std::string_view> v = {"a", "b", "c"};

    std::vector<std::string> r;
    view(v).to(&r);
    EXPECT_EQ(r, std::vector<std::string>({"a", "b", "c"}));
}
