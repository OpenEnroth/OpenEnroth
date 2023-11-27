#pragma once

#include <vector>
#include <any>
#include <utility>

class Config;
class AnyConfigEntry;

class EngineTraceConfigSnapshot {
 public:
    EngineTraceConfigSnapshot() = default;
    explicit EngineTraceConfigSnapshot(Config *config);

    void apply();

 private:
    std::vector<std::pair<AnyConfigEntry *, std::any>> _values;
};
