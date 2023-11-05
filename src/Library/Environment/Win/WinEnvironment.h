#pragma once

#include "Library/Environment/Interface/Environment.h"

#include <string>

class WinEnvironment : public Environment {
 public:
    virtual std::string queryRegistry(const std::string &path) const override;
    virtual std::string path(EnvironmentPath path) const override;
    virtual std::string getenv(const std::string &key) const override;
};
