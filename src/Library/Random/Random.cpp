#include "Random.h"

#include "MersenneTwisterRandomEngine.h"
#include "Library/Random/RandomEngine.h"

std::unique_ptr<RandomEngine> grng = std::make_unique<MersenneTwisterRandomEngine>();
std::unique_ptr<RandomEngine> vrng = std::make_unique<MersenneTwisterRandomEngine>();

