#pragma once

#include <span>
#include <utility>
#include <type_traits>

#include "Engine/Objects/Actor.h"

#include "TestController.h"

class ActorTapeRecorder {
 public:
    explicit ActorTapeRecorder(TestController *controller);

    template<class Callback, class T = std::decay_t<std::invoke_result_t<Callback, const Actor &>>>
    TestTape<T> custom(int actorIndex, Callback callback) {
        return _controller->recordTape([actorIndex, callback = std::move(callback)] {
            return callback(actors()[actorIndex]);
        });
    }

    TestTape<int> countByState(AIState state);

    TestTape<int> countByBuff(ACTOR_BUFF_INDEX buff);

    TestTape<int> hp(int actorIndex);

 private:
    static std::span<Actor> actors();

 private:
    TestController *_controller = nullptr;
};
