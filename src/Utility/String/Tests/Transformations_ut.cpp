#include <string>
#include <vector>
#include <set>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Transformations.h"

UNIT_TEST(StringTransformations, toHexDump) {
    EXPECT_EQ(toHexDump("1234", 2), "3132 3334");
    EXPECT_EQ(toHexDump("0000"), "30303030");
}

UNIT_TEST(StringTransformations, replaceAll) {
    EXPECT_EQ(replaceAll("123", "1", "123"), "12323");
    EXPECT_EQ(replaceAll("123", "10", "100"), "123");
    EXPECT_EQ(replaceAll("ab123ab", "ab", "zz"), "zz123zz");
    EXPECT_EQ(replaceAll("AAAA", "AA", "AAZAA"), "AAZAAAAZAA");
}

UNIT_TEST(StringTransformations, unquote) {
    // Quoted fields: outer quotes stripped, inner doubled quotes collapsed.
    EXPECT_EQ(unquote("\"hello\""), "hello");
    EXPECT_EQ(unquote("\"\""), "");
    EXPECT_EQ(unquote("\"say \"\"hi\"\"\""), "say \"hi\"");
    EXPECT_EQ(unquote("\"a, \"\"b\"\" and \"\"c\"\".\""), "a, \"b\" and \"c\".");

    // Unquoted fields are returned verbatim - the "" escaping doesn't apply.
    EXPECT_EQ(unquote("hello"), "hello");
    EXPECT_EQ(unquote("a\"\"b"), "a\"\"b");
    EXPECT_EQ(unquote(""), "");
}
