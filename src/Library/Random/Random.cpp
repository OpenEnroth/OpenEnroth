#include "Random.h"

#include "MersenneTwisterRandomEngine.h"

std::unique_ptr<RandomEngine> grng = std::make_unique<MersenneTwisterRandomEngine>();
std::unique_ptr<RandomEngine> vrng = std::make_unique<MersenneTwisterRandomEngine>();

