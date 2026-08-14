#pragma once

#include <string>
#include <vector>

#include "Utility/System/NativePath.h"

class Environment;

constexpr char mm6PathOverrideKey[] = "OPENENROTH_MM6_PATH";
constexpr char mm7PathOverrideKey[] = "OPENENROTH_MM7_PATH";
constexpr char mm8PathOverrideKey[] = "OPENENROTH_MM8_PATH";

std::vector<std::string> resolveMm6Paths(Environment *environment);
std::vector<std::string> resolveMm7Paths(Environment *environment);
std::vector<std::string> resolveMm8Paths(Environment *environment);

bool validateMm7Path(const NativePath &dataPath, std::string *missingFile);

std::string resolveMm7UserPath(Environment *environment);
