#include "RandomEngine.h"

#include <cassert>

int RandomEngine::RandomInSegment(int min, int max) {
    assert(max >= min);

    return min + Random(max - min + 1);
}

int RandomEngine::RandomDice(int count, int faces) {
    assert(count >= 0 && faces >= 0);

    if (count == 0 || faces == 0)
        return 0;

    int result = 0;
    for (int i = 0; i < count; i++)
        result += 1 + Random(faces);
    return result;
}
