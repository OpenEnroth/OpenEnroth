#include "ActorTapeRecorder.h"

#include <cassert>
#include <functional>

using namespace std::placeholders; // NOLINT

ActorTapeRecorder::ActorTapeRecorder(TestController *controller) : _controller(controller) {
    assert(controller);
}

std::span<Actor> ActorTapeRecorder::actors() {
    return pActors;
}

TestTape<int> ActorTapeRecorder::countByState(AIState state) {
    return _controller->recordTape([state] {
        return static_cast<int>(std::ranges::count(actors(), state, &Actor::aiState));
    });
}

TestTape<int> ActorTapeRecorder::countByBuff(ACTOR_BUFF_INDEX buff) {
    return _controller->recordTape([buff] {
        return static_cast<int>(std::ranges::count_if(actors(), [buff] (const Actor &actor) {
            return actor.buffs[buff].Active();
        }));
    });
}

TestTape<int> ActorTapeRecorder::hp(int actorIndex) {
    return custom(actorIndex, std::bind<int>(&Actor::currentHP, _1));
}

TestTape<AIState> ActorTapeRecorder::aiState(int actorIndex) {
    return custom(actorIndex, std::bind(&Actor::aiState, _1));
}

TestTape<bool> ActorTapeRecorder::hasBuff(int actorIndex, ACTOR_BUFF_INDEX buff) {
    return custom(actorIndex, [buff] (const Actor &actor) {
        return actor.buffs[buff].Active();
    });
}

