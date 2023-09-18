#include "RandomEngine.h"

#include <cassert>

#include "MersenneTwisterRandomEngine.h"
#include "SequentialRandomEngine.h"

std::unique_ptr<RandomEngine> RandomEngine::create(RandomEngineType type) {
    if (type == RANDOM_ENGINE_MERSENNE_TWISTER) {
        return std::make_unique<MersenneTwisterRandomEngine>();
    } else {
        assert(type == RANDOM_ENGINE_SEQUENTIAL);
        return std::make_unique<SequentialRandomEngine>();
    }
}

int RandomEngine::randomInSegment(int min, int max) {
    assert(max >= min);

    return min + random(max - min + 1);
}

int RandomEngine::randomDice(int count, int faces) {
    assert(count >= 0 && faces >= 0);

    if (count == 0 || faces == 0)
        return 0;

    int result = 0;
    for (int i = 0; i < count; i++)
        result += 1 + random(faces);
    return result;
}
