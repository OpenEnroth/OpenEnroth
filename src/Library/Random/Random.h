#pragma once

#include <memory>

#include "RandomEngine.h"

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

