#include "Random.h"

std::unique_ptr<RandomEngineFactory> rngf = RandomEngineFactory::standard();
std::unique_ptr<RandomEngine> grng = rngf->createEngine(RANDOM_ENGINE_MERSENNE_TWISTER);
std::unique_ptr<RandomEngine> vrng = rngf->createEngine(RANDOM_ENGINE_MERSENNE_TWISTER);
