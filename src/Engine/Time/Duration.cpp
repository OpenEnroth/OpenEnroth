#include "Duration.h"

#include "Library/Random/RandomEngine.h"

Duration Duration::randomRealtimeMilliseconds(RandomEngine *rng, int64_t hi) {
    return Duration::fromTicks(rng->random(Duration::fromRealtimeMilliseconds(hi).ticks()));
}

Duration Duration::randomRealtimeMilliseconds(RandomEngine *rng, int64_t lo, int64_t hi) {
    return Duration::fromRealtimeMilliseconds(lo) + Duration::randomRealtimeMilliseconds(rng, hi - lo);
}

Duration Duration::randomRealtimeSeconds(RandomEngine *rng, int64_t hi) {
    return Duration::fromTicks(rng->random(Duration::fromRealtimeSeconds(hi).ticks()));
}

Duration Duration::randomRealtimeSeconds(RandomEngine *rng, int64_t lo, int64_t hi) {
    return Duration::fromRealtimeSeconds(lo) + Duration::randomRealtimeSeconds(rng, hi - lo);
}

