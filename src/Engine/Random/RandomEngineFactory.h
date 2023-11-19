#pragma once

#include <memory>

#include "Library/Random/RandomEngine.h"

#include "RandomEnums.h"

class RandomEngineFactory {
 public:
    virtual ~RandomEngineFactory() = default;
    virtual std::unique_ptr<RandomEngine> createEngine(RandomEngineType engineType) = 0;

    static std::unique_ptr<RandomEngineFactory> standard();
    static std::unique_ptr<RandomEngineFactory> tracing();
};
