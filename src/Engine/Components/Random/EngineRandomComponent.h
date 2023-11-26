#pragma once

#include <memory>

#include "Engine/Random/RandomEnums.h"

#include "Library/Platform/Application/PlatformApplicationAware.h"

#include "Utility/IndexedArray.h"

class RandomEngine;
class Platform;

class EngineRandomComponent : public PlatformApplicationAware {
 public:
    EngineRandomComponent();
    ~EngineRandomComponent();

    [[nodiscard]] bool isTracing() const;
    void setTracing(bool tracing);

    // TODO(captainurist): reset doesn't reset tracing state. Think of a better name.
    void reset(RandomEngineType withType);

 private:
    virtual void installNotify() override;
    virtual void removeNotify() override;
    void swizzleGlobals();

 private:
    std::unique_ptr<RandomEngine> _grng;
    std::unique_ptr<RandomEngine> _vrng;
    std::unique_ptr<RandomEngine> _tracingGrng;
    bool _tracing = false;
};
