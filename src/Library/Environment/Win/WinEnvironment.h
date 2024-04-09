#pragma once

#include <string>

#include "Library/Environment/Interface/Environment.h"

class WinEnvironment : public Environment {
 public:
    [[nodiscard]] virtual std::string queryRegistry(const std::string &path) const override;
    [[nodiscard]] virtual std::string path(EnvironmentPath path) const override;
    [[nodiscard]] virtual std::string getenv(const std::string &key) const override;
    virtual void setenv(const std::string &key, const std::string &value) const override;
};
