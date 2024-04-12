#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Json/Json.h"

template<class T>
void TestRoundTrip(const T &value) {
    T tmp;
    Json json;

    tmp = T();
    from_json(Json(value), tmp);
    EXPECT_EQ(tmp, value);

    tmp = T();
    to_json(json, value);

    std::string jsonString = to_string(json);
    json = Json::parse(jsonString);

    from_json(json, tmp);
    EXPECT_EQ(tmp, value);
}

UNIT_TEST(JsonSerialization, Standard) {
    TestRoundTrip(5);
    TestRoundTrip(ULLONG_MAX);
    TestRoundTrip(LLONG_MIN);
    TestRoundTrip(0.0);
    TestRoundTrip(false);
    TestRoundTrip(std::string(""));
    TestRoundTrip(std::string("asdf"));
    TestRoundTrip(std::vector<int>());
    TestRoundTrip(std::vector<int>({1, 2, 3}));
}

UNIT_TEST(JsonSerialization, StandardExceptions) {
    std::string s;
    EXPECT_ANY_THROW(from_json(Json(5), s));
    EXPECT_ANY_THROW(from_json(Json(1.0), s));
    EXPECT_ANY_THROW(from_json(Json(), s));

    int i = 0;
    double d = 0;
    EXPECT_ANY_THROW(from_json(Json("5"), i));
    EXPECT_ANY_THROW(from_json(Json(), i));
    EXPECT_ANY_THROW(from_json(Json("5"), d));
    EXPECT_ANY_THROW(from_json(Json(), d));

    from_json(Json(1.1), i);
    EXPECT_EQ(i, 1);

    from_json(Json(10), d);
    EXPECT_EQ(d, 10);
}

struct TestPair {
    int a = 0;
    int b = 0;

    friend bool operator==(const TestPair &l, const TestPair &r) = default;
};

struct TestStruct {
    int i = 0;
    std::vector<TestPair> v;
    double d = 0;
    std::string s;

    friend bool operator==(const TestStruct &l, const TestStruct &r) = default;
};

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(TestPair, (
    (a, "a"),
    (b, "b")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(TestStruct, (
    (i, "i"),
    (v, "v"),
    (d, "d"),
    (s, "s")
))

UNIT_TEST(JsonSerialization, Struct) {
    TestStruct t1;
    t1.s = "asdf";
    t1.d = 1.0;
    t1.i = 10;
    t1.v.emplace_back(TestPair{1, 1});

    TestStruct t2;
    Json json;
    to_json(json, t1);
    from_json(json, t2);

    EXPECT_EQ(t1, t2);

    json = Json();
    EXPECT_ANY_THROW(from_json(json, t2));

    json = Json::object();
    EXPECT_NO_THROW(from_json(json, t2)); // All fields are optional

    json = Json::array();
    json.push_back(Json::object());
    json.push_back(Json());
    std::vector<TestStruct> tv;
    EXPECT_ANY_THROW(from_json(json, tv));
}

enum class TotallyBrokenEnum {
    VALUE_0 = 0,
    VALUE_1 = 1,
    VALUE_2 = 2
};
using enum TotallyBrokenEnum;

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(TotallyBrokenEnum, CASE_INSENSITIVE, {
    {VALUE_0, "000000000000000000"}
})

MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(TotallyBrokenEnum)

UNIT_TEST(JsonSerialization, Enum) {
    TestRoundTrip(VALUE_0);

    Json json;
    EXPECT_NO_THROW(to_json(json, VALUE_0));
    EXPECT_ANY_THROW(to_json(json, VALUE_1));

    TotallyBrokenEnum e = VALUE_0;
    EXPECT_ANY_THROW(from_json(Json("0"), e));

    // TODO(captainurist): nlohmann json just chokes on this! fix upstream & uncomment.
    std::vector<TotallyBrokenEnum> v;
    v.push_back(VALUE_1);
    // EXPECT_ANY_THROW(to_json(json, v));
}
