#pragma once

#include <string>

#include "Library/Environment/Interface/Environment.h"

class PosixEnvironment : public Environment {
 public:
    virtual std::string queryRegistry(const std::string &path) const override;
    virtual std::string path(EnvironmentPath path) const override;
    virtual std::string getenv(const std::string &key) const override;
};
