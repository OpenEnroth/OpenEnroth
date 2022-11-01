#include <numeric>
#include <vector>

#include "test/test.h"

#include "src/Utility/Segment.h"

TEST(Range, Iteration) {
    enum class Index { AA, BB, CC };
    using enum Index;

    int counter = 0;
    for(Index i : Segment(AA, CC))
        counter++;
    EXPECT_EQ(counter, 3);

    counter = 0;
    for(Index i : Segment(AA, AA))
        counter++;
    EXPECT_EQ(counter, 1);

    std::vector<int> v;
    for(int i : Segment(0, 99))
        v.push_back(i);
    std::vector<int> w(100, 0);
    std::iota(w.begin(), w.end(), 0);
    EXPECT_EQ(v, w);
}
