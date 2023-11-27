#pragma once

#include <memory>

#include "Library/Random/RandomEngine.h"

class EngineRandomComponent;

namespace detail {

class RandomEnginePtr {
 public:
    RandomEngine *operator->() const {
        return _ptr;
    }

    RandomEngine &operator*() const {
        return *_ptr;
    }

    RandomEngine *get() const {
        return _ptr;
    }

 private:
    friend EngineRandomComponent;

 private:
    RandomEngine *_ptr = nullptr;
};

} // namespace detail

/**
 * @file
 *
 * Some notes on what is happening here.
 *
 * We want to be able to record play traces, and for this we need deterministic random number generation. Unfortunately,
 * right now some code paths use `pMiscTimer` in conditionals, and this timer isn't even stored in saves. So there is
 * some non-determinism in the code, and there's little that we can do about it.
 *
 * This is the reason why we have two random endpoints. One is for everything that's affecting gameplay, the other one
 * is for everything else.
 */

/**
 * `grng` stands for `game random number generator`. This is the random number engine that should be used for
 * everything that affects gameplay.
 *
 * `grng` is managed externally by an instance of `EngineRandomComponent`. Create the component before using `grng`.
 */
extern detail::RandomEnginePtr grng;

/**
 * `vrng` stands for `view random number generator`. This is the random number engine that should be used for
 * visual & audio effects that do not affect gameplay, e.g. particle trails, or party members shouting "got him!"
 * after a kill.
 *
 * `vrng` is managed externally by an instance of `EngineRandomComponent`. Create the component before using `vrng`.
 */
extern detail::RandomEnginePtr vrng;

