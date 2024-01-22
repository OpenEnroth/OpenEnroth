#pragma once

#include <string>

#include "Library/Environment/Interface/Environment.h"

class PosixEnvironment : public Environment {
 public:
    std::string getenv(std::string_view key) const override;
 private:
    std::string path(EnvironmentPath path) const override;
    GamePaths getGamePaths(const PathResolutionConfig& config) const override;
};
