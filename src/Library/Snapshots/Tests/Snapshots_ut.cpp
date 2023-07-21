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

static const std::vector<int> ints012 = {0, 1, 2};
static const std::vector<int> ints345 = {3, 4, 5};
static const std::vector<int> ints012345 = {0, 1, 2, 3, 4, 5};

UNIT_TEST(Snapshots, Via) {
    Blob blob;
    std::vector<int> ref = {100, 200}; // Also test overwrite.
    serialize(ints012, &blob, via<Int_MM>());
    deserialize(blob, &ref, via<Int_MM>());
    EXPECT_EQ(ref, ints012);
}

UNIT_TEST(Snapshots, SizedVia) {
    Blob blob;
    std::vector<int> ref = {100, 200}; // Also test overwrite.
    serialize(ints012, &blob, unsized(), via<Int_MM>());
    deserialize(blob, &ref, presized(3), via<Int_MM>());
    EXPECT_EQ(ref, ints012);
}

UNIT_TEST(Snapshots, MultiSizedVia) {
    std::string string;
    StringOutputStream output(&string);
    std::vector<int> ref = {100, 200}; // Also test overwrite.
    serialize(ints012, &output, unsized(), via<Int_MM>());
    serialize(ints345, &output, unsized(), via<Int_MM>());
    MemoryInputStream input2(string.data(), string.size());
    deserialize(input2, &ref, presized(6), via<Int_MM>());
    EXPECT_EQ(ref, ints012345);
}

UNIT_TEST(Snapshots, AppendSizedVia) {
    std::string string;
    StringOutputStream output(&string);
    std::vector<int> ref;
    serialize(ints012, &output, unsized(), via<Int_MM>());
    serialize(ints345, &output, unsized(), via<Int_MM>());
    MemoryInputStream input3(string.data(), string.size());
    deserialize(input3, &ref, append(), presized(3), via<Int_MM>());
    deserialize(input3, &ref, append(), presized(3), via<Int_MM>());
    EXPECT_EQ(ref, ints012345);
}

UNIT_TEST(Snapshots, AppendVia) {
    std::string string;
    StringOutputStream output(&string);
    std::vector<int> ref;
    serialize(ints012, &output, via<Int_MM>());
    serialize(ints345, &output, via<Int_MM>());
    MemoryInputStream input4(string.data(), string.size());
    deserialize(input4, &ref, append(), via<Int_MM>());
    deserialize(input4, &ref, append(), via<Int_MM>());
    EXPECT_EQ(ref, ints012345);
}
