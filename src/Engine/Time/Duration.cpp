#include "Duration.h"

#include "Library/Random/RandomEngine.h"

Duration Duration::random(RandomEngine *rng, Duration hi) {
    return fromTicks(rng->random(hi.ticks()));
}

Duration Duration::randomRealtimeMilliseconds(RandomEngine *rng, int64_t hi) {
    return random(rng, fromRealtimeMilliseconds(hi));
}

Duration Duration::randomRealtimeMilliseconds(RandomEngine *rng, int64_t lo, int64_t hi) {
    return fromRealtimeMilliseconds(lo) + randomRealtimeMilliseconds(rng, hi - lo);
}

Duration Duration::randomRealtimeSeconds(RandomEngine *rng, int64_t hi) {
    return random(rng, fromRealtimeSeconds(hi));
}

Duration Duration::randomRealtimeSeconds(RandomEngine *rng, int64_t lo, int64_t hi) {
    return fromRealtimeSeconds(lo) + randomRealtimeSeconds(rng, hi - lo);
}

