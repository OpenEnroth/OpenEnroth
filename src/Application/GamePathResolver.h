#pragma once

#include <string_view>

#include "Library/Environment/Interface/Environment.h"

constexpr std::string_view mm6PathOverrideKey{ "OPENENROTH_MM6_PATH" };
constexpr std::string_view mm7PathOverrideKey{ "OPENENROTH_MM7_PATH" };
constexpr std::string_view mm8PathOverrideKey{ "OPENENROTH_MM8_PATH" };

Environment::GamePaths resolveMm6Paths(const Environment &environment);
Environment::GamePaths resolveMm7Paths(const Environment &environment);
Environment::GamePaths resolveMm8Paths(const Environment &environment);
