#include "RandomEngineFactory.h"

#include "Library/Random/MersenneTwisterRandomEngine.h"
#include "Library/Random/SequentialRandomEngine.h"

#include "TracingRandomEngine.h"

class StandardRandomEngineFactory : public RandomEngineFactory {
 public:
    virtual std::unique_ptr<RandomEngine> createEngine(RandomEngineType engineType) override {
        if (engineType == RANDOM_ENGINE_MERSENNE_TWISTER) {
            return std::make_unique<MersenneTwisterRandomEngine>();
        } else {
            assert(engineType == RANDOM_ENGINE_SEQUENTIAL);
            return std::make_unique<SequentialRandomEngine>();
        }
    }
};

class TracingRandomEngineFactory : public StandardRandomEngineFactory {
 public:
    virtual std::unique_ptr<RandomEngine> createEngine(RandomEngineType engineType) override {
        return std::make_unique<TracingRandomEngine>(StandardRandomEngineFactory::createEngine(engineType));
    };
};

std::unique_ptr<RandomEngineFactory> RandomEngineFactory::standard() {
    return std::make_unique<StandardRandomEngineFactory>();
}

std::unique_ptr<RandomEngineFactory> RandomEngineFactory::tracing() {
    return std::make_unique<TracingRandomEngineFactory>();
}
