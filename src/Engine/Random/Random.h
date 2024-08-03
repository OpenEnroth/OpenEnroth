#pragma once

#include <memory>

#include "Library/Random/RandomEngine.h"

/**
 * @file
 *
 * Some notes on what is happening here.
 *
 * We want to be able to record play traces, and for this we need deterministic random number generation. This is the
 * reason why we have two random endpoints. One is for everything that's affecting gameplay, the other one is for
 * everything else.
 */

/**
 * `grng` stands for `game random number generator`. This is the random number engine that should be used for
 * everything that affects gameplay.
 *
 * `grng` is managed externally by an instance of `EngineRandomComponent`. Create the component before using `grng`.
 */
extern RandomEngine *grng;

/**
 * `vrng` stands for `view random number generator`. This is the random number engine that should be used for
 * visual & audio effects that do not affect gameplay, e.g. particle trails, or party members shouting "got him!"
 * after a kill.
 *
 * `vrng` is managed externally by an instance of `EngineRandomComponent`. Create the component before using `vrng`.
 */
extern RandomEngine *vrng;
