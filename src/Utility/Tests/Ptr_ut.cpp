#include <memory>
#include <type_traits>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Ptr.h"

struct PtrTestBase {
    int value = 1;
};

struct PtrTestDerived : PtrTestBase {};

struct PtrTestIncomplete;

static_assert(std::is_trivially_copyable_v<Ptr<int>>);
static_assert(std::is_trivially_copyable_v<Ptr<PtrTestIncomplete>>);

static int readValue(Ptr<int> ptr) {
    return *ptr;
}

static long useCountInside(Ptr<int> ptr, const std::shared_ptr<int> &holder) {
    return holder.use_count();
}

UNIT_TEST(Ptr, Null) {
    Ptr<int> a;
    Ptr<int> b = nullptr;
    Ptr<int> c = std::shared_ptr<int>();
    Ptr<int> d = std::unique_ptr<int>();

    EXPECT_EQ(a.get(), nullptr);
    EXPECT_EQ(b.get(), nullptr);
    EXPECT_EQ(c.get(), nullptr);
    EXPECT_EQ(d.get(), nullptr);
}

UNIT_TEST(Ptr, Raw) {
    int value = 5;
    Ptr<int> ptr = &value;

    EXPECT_EQ(ptr.get(), &value);
    EXPECT_EQ(*ptr, 5);
}

UNIT_TEST(Ptr, SharedPtr) {
    std::shared_ptr<int> holder = std::make_shared<int>(5);
    Ptr<int> ptr = holder;

    EXPECT_EQ(ptr.get(), holder.get());
    EXPECT_EQ(*ptr, 5);
}

UNIT_TEST(Ptr, UniquePtr) {
    std::unique_ptr<int> holder = std::make_unique<int>(5);
    Ptr<int> ptr = holder;

    EXPECT_EQ(ptr.get(), holder.get());
    EXPECT_EQ(*ptr, 5);
}

UNIT_TEST(Ptr, DerivedToBase) {
    PtrTestDerived value;
    Ptr<PtrTestBase> raw = &value;
    EXPECT_EQ(raw.get(), &value);

    std::shared_ptr<PtrTestDerived> shared = std::make_shared<PtrTestDerived>();
    Ptr<PtrTestBase> fromShared = shared;
    EXPECT_EQ(fromShared.get(), shared.get());

    std::unique_ptr<PtrTestDerived> unique = std::make_unique<PtrTestDerived>();
    Ptr<PtrTestBase> fromUnique = unique;
    EXPECT_EQ(fromUnique.get(), unique.get());
}

UNIT_TEST(Ptr, Accessors) {
    PtrTestBase value;
    Ptr<PtrTestBase> ptr = &value;

    EXPECT_EQ(ptr->value, 1);
    EXPECT_EQ((*ptr).value, 1);
    EXPECT_EQ(&*ptr, &value);

    PtrTestBase *raw = ptr;
    EXPECT_EQ(raw, &value);
}

UNIT_TEST(Ptr, Comparisons) {
    int value = 5;
    Ptr<int> ptr = &value;
    Ptr<int> null;

    EXPECT_TRUE(ptr ? true : false);
    EXPECT_FALSE(null ? true : false);
    EXPECT_FALSE(!ptr);
    EXPECT_TRUE(!null);
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_TRUE(null == nullptr);
    EXPECT_TRUE(nullptr == null);
    EXPECT_TRUE(ptr == &value);
    EXPECT_TRUE(ptr == Ptr<int>(&value));
    EXPECT_TRUE(ptr != null);
}

UNIT_TEST(Ptr, IncompleteType) {
    std::shared_ptr<PtrTestIncomplete> shared;
    Ptr<PtrTestIncomplete> ptr = shared;

    EXPECT_TRUE(ptr == nullptr);
}

UNIT_TEST(Ptr, UseCountDoesNotChange) {
    std::shared_ptr<int> holder = std::make_shared<int>(5);

    EXPECT_EQ(useCountInside(holder, holder), 1);
    EXPECT_EQ(holder.use_count(), 1);
}

UNIT_TEST(Ptr, TemporaryArgument) {
    EXPECT_EQ(readValue(std::make_shared<int>(5)), 5);
    EXPECT_EQ(readValue(std::make_unique<int>(6)), 6);
}
