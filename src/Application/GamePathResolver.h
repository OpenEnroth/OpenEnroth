#pragma once

#include <string>
#include <string_view>
#include <vector>

class Environment;

constexpr std::string_view mm6PathOverrideKey{ "OPENENROTH_MM6_PATH" };
constexpr std::string_view mm7PathOverrideKey{ "OPENENROTH_MM7_PATH" };
constexpr std::string_view mm8PathOverrideKey{ "OPENENROTH_MM8_PATH" };

std::vector<std::string> resolveMm6Paths(const Environment &environment);
std::vector<std::string> resolveMm7Paths(const Environment &environment);
std::vector<std::string> resolveMm8Paths(const Environment &environment);
