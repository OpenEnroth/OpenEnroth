#pragma once

#include <memory>

#include "Library/Random/RandomEngine.h"

class Platform;

class TracingRandomEngine : public RandomEngine {
 public:
    explicit TracingRandomEngine(Platform *platform, RandomEngine *base);

    virtual float randomFloat() override;
    virtual int random(int hi) override;
    virtual int peek(int hi) const override;
    virtual void seed(int seed) override;

 private:
    template<class T>
    void printTrace(const char *function, const T &value) const;

 private:
    Platform *_platform = nullptr;
    RandomEngine *_base = nullptr;
};
