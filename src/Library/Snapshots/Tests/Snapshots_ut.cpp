#include "Testing/Unit/UnitTest.h"

#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/StringOutputStream.h"
#include "Utility/Streams/MemoryInputStream.h"

struct Int_MM {
    int dummy;
    int value;
};
MM_DECLARE_MEMCOPY_SERIALIZABLE(Int_MM)

void snapshot(int src, Int_MM *dst) {
    dst->dummy = 0;
    dst->value = src;
}

void reconstruct(const Int_MM &src, int *dst) {
    *dst = src.value;
}

UNIT_TEST(Snapshots, Serialization) {
    std::vector<int> ints012 = {0, 1, 2};
    std::vector<int> ints345 = {3, 4, 5};
    std::vector<int> ints012345 = {0, 1, 2, 3, 4 ,5};

    // Simple via.
    Blob blob0;
    std::vector<int> ref0;
    serialize(ints012, &blob0, via<Int_MM>());
    deserialize(blob0, &ref0, via<Int_MM>());
    EXPECT_EQ(ref0, ints012);

    // Unsized + via.
    Blob blob1;
    std::vector<int> ref1;
    serialize(ints012, &blob1, unsized(), via<Int_MM>());
    deserialize(blob1, &ref1, presized(3), via<Int_MM>());
    EXPECT_EQ(ref1, ints012);

    // Multiple unsized + via.
    std::string string2;
    StringOutputStream output2(&string2);
    std::vector<int> ref2;
    serialize(ints012, &output2, unsized(), via<Int_MM>());
    serialize(ints345, &output2, unsized(), via<Int_MM>());
    MemoryInputStream input2(string2.data(), string2.size());
    deserialize(input2, &ref2, presized(6), via<Int_MM>());
    EXPECT_EQ(ref2, ints012345);

    // append + presized + via.
    std::string string3;
    StringOutputStream output3(&string3);
    std::vector<int> ref3;
    serialize(ints012, &output3, unsized(), via<Int_MM>());
    serialize(ints345, &output3, unsized(), via<Int_MM>());
    MemoryInputStream input3(string3.data(), string3.size());
    deserialize(input3, &ref3, append(), presized(3), via<Int_MM>());
    deserialize(input3, &ref3, append(), presized(3), via<Int_MM>());
    EXPECT_EQ(ref3, ints012345);

    // append + via
    std::string string4;
    StringOutputStream output4(&string4);
    std::vector<int> ref4;
    serialize(ints012, &output4, via<Int_MM>());
    serialize(ints345, &output4, via<Int_MM>());
    MemoryInputStream input4(string4.data(), string4.size());
    deserialize(input4, &ref4, append(), via<Int_MM>());
    deserialize(input4, &ref4, append(), via<Int_MM>());
    EXPECT_EQ(ref4, ints012345);
}
