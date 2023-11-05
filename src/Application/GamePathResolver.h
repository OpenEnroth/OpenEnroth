#include <string>

class Environment;

constexpr char mm6PathOverrideKey[] = "OPENENROTH_MM6_PATH";
constexpr char mm7PathOverrideKey[] = "OPENENROTH_MM7_PATH";
constexpr char mm8PathOverrideKey[] = "OPENENROTH_MM8_PATH";

std::string resolveMm6Path(Environment *environment);
std::string resolveMm7Path(Environment *environment);
std::string resolveMm8Path(Environment *environment);
