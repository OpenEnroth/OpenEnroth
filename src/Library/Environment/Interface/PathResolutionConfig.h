#pragma once

#include <string_view>

struct PathResolutionConfig {
    using RegistryKeys = std::initializer_list<std::string_view>;

    std::string_view overrideEnvKey;
    RegistryKeys registryKeys;
};