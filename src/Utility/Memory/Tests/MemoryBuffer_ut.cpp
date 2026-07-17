#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/MemoryBuffer.h"

UNIT_TEST(MemoryBuffer, Empty) {
    MemoryBuffer buffer;

    std::span<const char> result = buffer.store(std::span<const char>());
    EXPECT_TRUE(result.empty());
}

UNIT_TEST(MemoryBuffer, StoreMakesACopy) {
    MemoryBuffer buffer;

    std::string data = "some data";
    std::span<const char> result = buffer.store(std::span<const char>(data));

    EXPECT_NE(result.data(), data.data());
    EXPECT_EQ(std::string(result.begin(), result.end()), data);

    data = "xxxx xxxx"; // Stored copy is unaffected.
    EXPECT_EQ(std::string(result.begin(), result.end()), "some data");
}

UNIT_TEST(MemoryBuffer, StoreIsPointerStable) {
    // Store enough data to span several storage chunks, and check that earlier views stay valid.
    MemoryBuffer buffer;

    std::vector<std::span<const char>> views;
    for (int i = 0; i < 64; i++) {
        std::string data(256, static_cast<char>(i + 1));
        views.push_back(buffer.store(std::span<const char>(data)));
    }

    for (int i = 0; i < 64; i++) {
        EXPECT_EQ(views[i].size(), 256);
        EXPECT_EQ(views[i].front(), static_cast<char>(i + 1));
        EXPECT_EQ(views[i].back(), static_cast<char>(i + 1));
    }
}

UNIT_TEST(MemoryBuffer, StoreOverAligned) {
    struct alignas(64) OverAligned {
        uint64_t value = 0;
    };

    MemoryBuffer buffer;

    std::vector<std::span<const OverAligned>> views;
    for (int i = 0; i < 16; i++) {
        std::vector<OverAligned> data(i);
        for (int j = 0; j < i; j++)
            data[j].value = i * 100 + j;

        std::string chars(1, 'x');
        (void) buffer.store(std::span<const char>(chars)); // Knock the write cursor out of alignment.
        views.push_back(buffer.store(std::span<const OverAligned>(data)));
    }

    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(reinterpret_cast<uintptr_t>(views[i].data()) % alignof(OverAligned), 0);
        EXPECT_EQ(views[i].size(), i);
        for (int j = 0; j < i; j++)
            EXPECT_EQ(views[i][j].value, i * 100 + j);
    }
}

UNIT_TEST(MemoryBuffer, StoreIsAligned) {
    // Interleave stores of differently-sized types, and check that the returned views are properly aligned.
    MemoryBuffer buffer;

    for (int i = 0; i < 16; i++) {
        std::string chars(i, 'x'); // Zero-size stores for i == 0 are also OK.
        std::vector<uint64_t> ints(i, 0x0123456789ABCDEFull);

        std::span<const char> charsView = buffer.store(std::span<const char>(chars));
        std::span<const uint64_t> intsView = buffer.store(std::span<const uint64_t>(ints));

        EXPECT_EQ(reinterpret_cast<uintptr_t>(intsView.data()) % alignof(uint64_t), 0);
        EXPECT_EQ(std::string(charsView.begin(), charsView.end()), chars);
        for (uint64_t value : intsView)
            EXPECT_EQ(value, 0x0123456789ABCDEFull);
    }
}
