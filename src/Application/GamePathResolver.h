#pragma once

#include <string>
#include <vector>

#include "Library/Environment/Interface/Environment.h"

constexpr std::string_view mm6PathOverrideKey{ "OPENENROTH_MM6_PATH" };
constexpr std::string_view mm7PathOverrideKey{ "OPENENROTH_MM7_PATH" };
constexpr std::string_view mm8PathOverrideKey{ "OPENENROTH_MM8_PATH" };

std::vector<std::string> resolveMm6Paths(Environment *environment);
std::vector<std::string> resolveMm7Paths(Environment *environment);
std::vector<std::string> resolveMm8Paths(Environment *environment);
