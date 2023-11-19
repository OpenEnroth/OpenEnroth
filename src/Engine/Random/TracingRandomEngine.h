#pragma once

#include <memory>

#include "Library/Random/RandomEngine.h"

class TracingRandomEngine : public RandomEngine {
 public:
    explicit TracingRandomEngine(std::unique_ptr<RandomEngine> base);

    virtual float randomFloat() override;
    virtual int random(int hi) override;
    virtual int peek(int hi) const override;
    virtual void seed(int seed) override;

 private:
    template<class T>
    void printTrace(const char *function, const T &value) const;

 private:
    std::unique_ptr<RandomEngine> _base;
};
