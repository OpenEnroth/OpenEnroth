#include "Random.h"

#include <memory>
#include <utility>

#include "MersenneTwisterRandomEngine.h"

static std::unique_ptr<RandomEngine> StaticGlobalRandomEngine;

RandomEngine *GlobalRandomEngine() {
    if (!StaticGlobalRandomEngine) {
        // We don't guarantee thread safety, so this code is perfectly OK, and it rids us of potential problems with
        // static initialization order.
        StaticGlobalRandomEngine = std::make_unique<MersenneTwisterRandomEngine>();
    }

    return StaticGlobalRandomEngine.get();
}

void SetGlobalRandomEngine(std::unique_ptr<RandomEngine> engine) {
    assert(engine);

    StaticGlobalRandomEngine = std::move(engine);
}

void SeedRandom(int seed) {
    GlobalRandomEngine()->Seed(seed);
}

int Random(int hi) {
    assert(hi > 0);

    return GlobalRandomEngine()->Random(hi);
}

int Random(int lo, int hi) {
    assert(hi > lo);

    return lo + GlobalRandomEngine()->Random(hi - lo);
}

float RandomFloat() {
    uint32_t value = GlobalRandomEngine()->Random();

    return static_cast<double>(value) / (static_cast<double>(UINT32_MAX) + 1);
}

int RandomDice(int count, int faces) {
    assert(count >= 0 && faces >= 0);

    if (count == 0 || faces == 0)
        return 0;

    RandomEngine *engine = GlobalRandomEngine();

    int result = 0;
    for (int i = 0; i < count; i++)
        result += 1 + engine->Random(faces);
    return result;
}
