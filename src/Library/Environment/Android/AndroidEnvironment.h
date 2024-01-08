#pragma once

#include "Library/Environment/Interface/Environment.h"

class AndroidEnvironment : public Environment {
public:
    virtual std::string queryRegistry(std::string_view path) const override;
    virtual std::string path(EnvironmentPath path) const override;
    virtual std::string getenv(std::string_view key) const override;
};
