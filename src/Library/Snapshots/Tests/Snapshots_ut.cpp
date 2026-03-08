#include <cstdint>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

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

struct Offset_MM {
    int value;
};
MM_DECLARE_MEMCOPY_SERIALIZABLE(Offset_MM)

void snapshot(int src, Offset_MM *dst, ContextTag<int> offset) {
    dst->value = src + *offset;
}

void reconstruct(const Offset_MM &src, int *dst, ContextTag<int> offset) {
    *dst = src.value - *offset;
}

static const std::vector<int> ints012 = {0, 1, 2};
static const std::vector<int> ints345 = {3, 4, 5};
static const std::vector<int> ints012345 = {0, 1, 2, 3, 4, 5};

UNIT_TEST(Snapshots, Via) {
    Blob blob;
    std::vector<int> ref = {100, 200}; // Also test overwrite.
    serialize(ints012, &blob, tags::each, tags::via<Int_MM>);
    deserialize(blob, &ref, tags::each, tags::via<Int_MM>);
    EXPECT_EQ(ref, ints012);
}

UNIT_TEST(Snapshots, SizedVia) {
    Blob blob;
    std::vector<int> ref = {100, 200}; // Also test overwrite.
    serialize(ints012, &blob, tags::unsized, tags::each, tags::via<Int_MM>);
    deserialize(blob, &ref, tags::presized(3), tags::each, tags::via<Int_MM>);
    EXPECT_EQ(ref, ints012);
}

UNIT_TEST(Snapshots, MultiSizedVia) {
    Blob blob;
    BlobOutputStream output(&blob);
    std::vector<int> ref = {100, 200}; // Also test overwrite.
    serialize(ints012, &output, tags::unsized, tags::each, tags::via<Int_MM>);
    serialize(ints345, &output, tags::unsized, tags::each, tags::via<Int_MM>);
    output.close();
    BlobInputStream input2(std::move(blob));
    deserialize(input2, &ref, tags::presized(6), tags::each, tags::via<Int_MM>);
    EXPECT_EQ(ref, ints012345);
}

UNIT_TEST(Snapshots, AppendSizedVia) {
    Blob blob;
    BlobOutputStream output(&blob);
    std::vector<int> ref;
    serialize(ints012, &output, tags::unsized, tags::each, tags::via<Int_MM>);
    serialize(ints345, &output, tags::unsized, tags::each, tags::via<Int_MM>);
    output.close();
    BlobInputStream input3(std::move(blob));
    deserialize(input3, &ref, tags::append, tags::presized(3), tags::each, tags::via<Int_MM>);
    deserialize(input3, &ref, tags::append, tags::presized(3), tags::each, tags::via<Int_MM>);
    EXPECT_EQ(ref, ints012345);
}

UNIT_TEST(Snapshots, AppendVia) {
    Blob blob;
    BlobOutputStream output(&blob);
    std::vector<int> ref;
    serialize(ints012, &output, tags::each, tags::via<Int_MM>);
    serialize(ints345, &output, tags::each, tags::via<Int_MM>);
    output.close();
    BlobInputStream input4(std::move(blob));
    deserialize(input4, &ref, tags::append, tags::each, tags::via<Int_MM>);
    deserialize(input4, &ref, tags::append, tags::each, tags::via<Int_MM>);
    EXPECT_EQ(ref, ints012345);
}

UNIT_TEST(Snapshots, ArrayVia) {
    const std::array<int, 3> ints123 = {1, 2, 3};

    Blob blob;
    std::array<int, 3> ref;
    serialize(ints123, &blob, tags::each, tags::via<Int_MM>);
    deserialize(blob, &ref, tags::each, tags::via<Int_MM>);
    EXPECT_EQ(ref, ints123);
}

UNIT_TEST(Snapshots, ViaWithTags) {
    Blob blob;
    int ref = 0;

    serialize(10, &blob, tags::via<Offset_MM>, tags::context(100));
    deserialize(blob, &ref, tags::via<Offset_MM>, tags::context(100));
    EXPECT_EQ(ref, 10);
}

UNIT_TEST(Snapshots, IndexedBitset) {
    std::array<uint8_t, 4> bytes = {255, 1, 128, 0};

    IndexedBitset<1, 32> bits;
    bits.fill(0);
    bits[1] = 1;
    bits[2] = 1;
    bits[3] = 1;
    bits[4] = 1;
    bits[5] = 1;
    bits[6] = 1;
    bits[7] = 1;
    bits[8] = 1;
    bits[16] = 1;
    bits[17] = 1;

    std::array<uint8_t, 4> bytes2;
    IndexedBitset<1, 32> bits2;

    snapshot(bits, &bytes2, tags::reverseBits);
    reconstruct(bytes, &bits2, tags::reverseBits);

    EXPECT_EQ(bytes, bytes2);
    EXPECT_EQ(bits, bits2);
}

UNIT_TEST(Snapshots, Cast) {
    uint32_t i32 = 0;
    uint8_t i8 = 0;

    i32 = 0xFFFF;
    snapshot(i32, &i8, tags::cast<uint32_t, uint8_t>);
    EXPECT_EQ(i8, 0xFF);

    reconstruct(i8, &i32, tags::cast<uint8_t, uint32_t>);
    EXPECT_EQ(i32, 0xFF);
}

template<class T1, class T2>
bool snapshotCastCompiles() {
    return requires (T1 a, T2 b) { snapshot(a, &b, tags::cast<uint8_t, uint32_t>); }; // NOLINT: linter chokes here.
}

UNIT_TEST(Snapshots, CastRequiresExplicitTypes) {
    // Check that calls with tags::cast<T1, T2> don't compile if the types passed into the function don't exactly match
    // the types passed as template parameters to tags::cast.
    EXPECT_FALSE((snapshotCastCompiles<int, int>()));
    EXPECT_FALSE((snapshotCastCompiles<uint8_t, int>()));
    EXPECT_FALSE((snapshotCastCompiles<int, uint32_t>()));
    EXPECT_FALSE((snapshotCastCompiles<int8_t, uint32_t>()));
    EXPECT_FALSE((snapshotCastCompiles<uint8_t, int32_t>()));
    EXPECT_TRUE((snapshotCastCompiles<uint8_t, uint32_t>()));
}


