#include <gtest/gtest.h>
#include <limits.h>
#include <string>
#include "Testing/Unit/UnitTest.h"
#include "Library/Serialization/Serialization.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Serialization/EnumSerializer.h"
#include "Utility/Flags.h"

UNIT_TEST(Serialization, Standard) {
    EXPECT_EQ(fromString<int>(toString(0)), 0);
    EXPECT_EQ(fromString<int>(toString(INT_MAX)), INT_MAX);
    EXPECT_EQ(fromString<int>(toString(INT_MIN)), INT_MIN);
    EXPECT_EQ(fromString<unsigned int>(toString(UINT_MAX)), UINT_MAX);
    EXPECT_EQ(fromString<unsigned  long long>(toString(ULLONG_MAX)), ULLONG_MAX);

    EXPECT_EQ(fromString<bool>(toString(true)), true);
    EXPECT_EQ(fromString<bool>(toString(false)), false);
    EXPECT_EQ(fromString<bool>("0"), false);
    EXPECT_EQ(fromString<bool>("1"), true);

    EXPECT_EQ(fromString<int>("0100"), 100);
    EXPECT_EQ(fromString<int>(std::string(100, '0') + toString(INT_MAX)), INT_MAX);

    EXPECT_ANY_THROW(fromString<bool>("da"));
    EXPECT_ANY_THROW(fromString<bool>(""));
    EXPECT_ANY_THROW(fromString<int>(""));
    EXPECT_ANY_THROW(fromString<int>(" 100"));
    EXPECT_ANY_THROW(fromString<int>("100 "));
    EXPECT_ANY_THROW(fromString<int>("+100"));
    EXPECT_ANY_THROW(fromString<int>("0x200"));
    EXPECT_ANY_THROW(fromString<int>("0b100"));
}

enum class TestEnum {
    VALUE_1 = 1,
    VALUE_2 = 2,
    VALUE_3 = 3,

    FLAGS_00FF = 0x00FF,
    FLAGS_0FF0 = 0x0FF0,
    FLAGS_FF00 = 0xFF00,
    FLAGS_F00F = 0xF00F,

    ALL_FLAGS = 0xFFFF,

    UNSERIALIZABLE = 0x11111111
};
using enum TestEnum;

MM_DECLARE_FLAGS(TestFlags, TestEnum)
MM_DECLARE_OPERATORS_FOR_FLAGS(TestFlags)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(TestEnum, CASE_SENSITIVE, {
    {VALUE_1, "1"},
    {VALUE_2, "2"},
    {VALUE_3, "3"},
    {FLAGS_00FF, "00FF"},
    {FLAGS_0FF0, "0FF0"},
    {FLAGS_FF00, "FF00"},
    {FLAGS_F00F, "F00F"},
    {ALL_FLAGS, "FFFF"},

    {FLAGS_00FF, "FIRST"},
    {FLAGS_0FF0, "SECOND"},
    {FLAGS_FF00, "THIRD"},
    {ALL_FLAGS, "ZUGZUG"}
})
MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(TestFlags)

UNIT_TEST(Serialization, Enum) {
    EXPECT_EQ(fromString<TestEnum>(toString(VALUE_1)), VALUE_1);

    EXPECT_EQ(toString(TestFlags()), "0");
    EXPECT_EQ(fromString<TestFlags>("0"), TestFlags());
    EXPECT_EQ(fromString<TestFlags>(toString(VALUE_1 | VALUE_2)), VALUE_3);
    EXPECT_EQ(fromString<TestFlags>(toString(FLAGS_F00F | FLAGS_00FF)), FLAGS_F00F | FLAGS_00FF);

    EXPECT_EQ(toString(ALL_FLAGS), "FFFF");
    EXPECT_EQ(toString(FLAGS_FF00 | FLAGS_0FF0 | VALUE_1), "1|0FF0|FF00");
    EXPECT_EQ(fromString<TestFlags>(" 1 | 0FF0 | FF00 "), FLAGS_FF00 | FLAGS_0FF0 | VALUE_1);
    EXPECT_EQ(fromString<TestFlags>(" 1 | 1 | 1 | 1 | 1"), VALUE_1);
    EXPECT_EQ(fromString<TestFlags>(" 1 "), VALUE_1);

    EXPECT_ANY_THROW(toString(UNSERIALIZABLE));
    EXPECT_ANY_THROW(toString(TestFlags(UNSERIALIZABLE)));
    EXPECT_ANY_THROW(fromString<TestEnum>(" 1"));
    EXPECT_ANY_THROW(fromString<TestEnum>("1 "));
    EXPECT_ANY_THROW(fromString<TestFlags>(" 1 2 "));
}

UNIT_TEST(Serialization, EnumMultiString) {
    EXPECT_EQ(fromString<TestEnum>("FIRST"), FLAGS_00FF);
    EXPECT_EQ(toString(fromString<TestFlags>("FIRST | SECOND | THIRD")), "FFFF");
    EXPECT_EQ(fromString<TestFlags>("ZUGZUG"), ALL_FLAGS);
}

enum class TestEnum2 {
    TEST_0 = 0,
    TEST_1 = 1,
    TEST_2 = 2,
    TEST_3 = 3,
    TEST_4 = 4,
};
using enum TestEnum2;
MM_DECLARE_FLAGS(TestFlags2, TestEnum2)
MM_DECLARE_OPERATORS_FOR_FLAGS(TestFlags2)

namespace detail {
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(TestEnum2, CASE_INSENSITIVE, {
    {TEST_0, "TEST_0"},
    {TEST_1, "TEST_1"},
    {TEST_2, "TEST_2"},
    {TEST_3, "TEST_3"},
    {TEST_4, "TEST_4"}
})
MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(TestFlags2)
} // namespace detail

UNIT_TEST(Serialization, CaseInsensitiveEnumWithNamespace) {
    std::string s;
    TestEnum2 e = TEST_0;
    TestFlags2 f = 0;

    EXPECT_TRUE(detail::trySerialize(TEST_0, &s));
    EXPECT_EQ(s, "TEST_0");

    EXPECT_TRUE(detail::tryDeserialize("teSt_1", &e));
    EXPECT_EQ(e, TEST_1);

    EXPECT_TRUE(detail::trySerialize(TestFlags2(), &s));
    EXPECT_EQ(s, "TEST_0");

    EXPECT_TRUE(detail::trySerialize(TEST_4 | TEST_1, &s));
    EXPECT_EQ(s, "TEST_1|TEST_4");

    EXPECT_TRUE(detail::tryDeserialize("test_4 | teST_1", &f));
    EXPECT_EQ(f, TEST_4 | TEST_1);
}

enum class BrokenFlag0 {
    BROKEN_FLAG_123 = 123,
};
using enum BrokenFlag0;

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(BrokenFlag0, CASE_SENSITIVE, {
    {BROKEN_FLAG_123, "0"}
})

enum class BrokenFlag1 {
    BROKEN_FLAG_1 = 1,
    BROKEN_FLAG_2 = 2,
    BROKEN_FLAG_3 = 3,
};
using enum BrokenFlag1;

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(BrokenFlag1, CASE_SENSITIVE, {
    {BROKEN_FLAG_1, "LOL"},
    {BROKEN_FLAG_2, "WUT"},
    {BROKEN_FLAG_3, "LOL|WUT"} // Ye, don't do that
})

enum class BrokenFlag2 {
    BROKEN_FLAG_OOOF = 1
};
using enum BrokenFlag2;

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(BrokenFlag2, CASE_SENSITIVE, {
    {BROKEN_FLAG_OOOF, " S P A C E S "}
})

enum class NonBrokenFlag1 {
    NON_BROKEN_FLAG_0 = 0
};
using enum NonBrokenFlag1;

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(NonBrokenFlag1, CASE_SENSITIVE, {
    {NON_BROKEN_FLAG_0, "0"}
})

UNIT_TEST(Serialization, BrokenFlags) {
    BrokenFlag0 b0;
    BrokenFlag1 b1;
    BrokenFlag2 b2;
    NonBrokenFlag1 nb1;

    EXPECT_FALSE(serializer(&b0)->isUsableWithFlags());
    EXPECT_FALSE(serializer(&b1)->isUsableWithFlags());
    EXPECT_FALSE(serializer(&b2)->isUsableWithFlags());
    EXPECT_TRUE(serializer(&nb1)->isUsableWithFlags());
}

enum class MagicEnum {
    MAGIC_A = 0,
    MAGIC_B = 1,
    MAGIC_C = 2,
    MAGIC_D = 3,
    MAGIC_E = 4,
};
using enum MagicEnum;

MM_DEFINE_ENUM_MAGIC_SERIALIZATION_FUNCTIONS(MagicEnum)

UNIT_TEST(Serialization, MagicEnum) {
    EXPECT_EQ(toString(MAGIC_A), "MAGIC_A");
    EXPECT_EQ(fromString<MagicEnum>(toString(MAGIC_B)), MAGIC_B);
    EXPECT_EQ(fromString<MagicEnum>("MAGIC_C"), MAGIC_C);

    EXPECT_ANY_THROW(fromString<MagicEnum>("MAGIC_X"));
    EXPECT_ANY_THROW(fromString<MagicEnum>("magic_a"));
    EXPECT_ANY_THROW(toString(static_cast<MagicEnum>(5)));
}
