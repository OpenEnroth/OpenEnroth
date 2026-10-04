#pragma once

#include <string>
#include <vector>

#include "Utility/System/Path.h"

class Environment;

constexpr char mm6PathOverrideKey[] = "OPENENROTH_MM6_PATH";
constexpr char mm7PathOverrideKey[] = "OPENENROTH_MM7_PATH";
constexpr char mm8PathOverrideKey[] = "OPENENROTH_MM8_PATH";

std::vector<Path> resolveMm6Paths(Environment *environment);
std::vector<Path> resolveMm7Paths(Environment *environment);
std::vector<Path> resolveMm8Paths(Environment *environment);

bool validateMm7Path(const Path &dataPath, std::string *missingFile);

Path resolveMm7UserPath(Environment *environment);
