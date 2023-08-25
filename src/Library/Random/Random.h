#pragma once

#include <memory>
#include "RandomEngine.h"

class RandomEngine;

// TODO(captaiurist): actually these static vars belong to Engine/

/**
 * @file
 *
 * Some notes on what is happening here.
 *
 * We want to be able to record play traces, and for this we need deterministic random number generation. Unfortunately,
 * right now some code paths use `pMiscTimer` in conditionals, and this timer isn't even stored in saves. So there is
 * some non-determinism in the code, and there's little that we can do about it.
 *
 * This is the reason why we have two random engines. One is for everything that's affecting gameplay, the other one
 * is for everything else.
 *
 * If there comes a moment where we'd want to e.g. test that characters are using the right speech phrases when killing
 * a monster, the proper way to do it would be to split the `vrng` engine in two. One for speech, one for everything
 * else. This won't add much overhead, and would let us test what we want to.
 *
 * The problems might start if we were to change game logic and introduce additional random calls for `grng` in the
 * code. But the goal of OpenEnroth is to keep the vanilla behavior, so this either shouldn't happen, or should
 * happen under a config flag. Thus, already recorded traces shouldn't break.
 */

/**
 * `grng` stands for `game random number generator`. This is the random number engine that should be used for
 * everything that affects gameplay.
 */
extern std::unique_ptr<RandomEngine> grng;

/**
 * `vrng` stands for `view random number generator`. This is the random number engine that should be used for
 * visual & audio effects that do not affect gameplay, e.g. particle trails, or party members shouting "got him!"
 * after a kill.
 */
extern std::unique_ptr<RandomEngine> vrng;
