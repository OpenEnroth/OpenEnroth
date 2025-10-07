#include "Testing/Unit/UnitTest.h"

#include "Utility/IndexedBitset.h"

UNIT_TEST(IndexedBitset, All) {
    enum class Index { A = 1000, B, C, D };
    using enum Index;

    IndexedBitset<A, D> a;
    a[A] = true;
    EXPECT_TRUE(a[A]);
    EXPECT_FALSE(a[B]);
    a.reset();
    EXPECT_FALSE(a[A]);
    a.set();
    EXPECT_TRUE(a[A]);
    a.fill(false);
    EXPECT_FALSE(a[A]);

    IndexedBitset<B, B> b;
    EXPECT_ANY_THROW((void) b.test(A));
    EXPECT_ANY_THROW(b.set(A, true));
    EXPECT_ANY_THROW((void) b.test(C));
    EXPECT_ANY_THROW(b.set(C, true));

    IndexedBitset<1, 2> c;
    EXPECT_FALSE(c[1]);
    EXPECT_ANY_THROW((void) c.test(0));
}
