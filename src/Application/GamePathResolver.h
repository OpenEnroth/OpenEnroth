#include <string>


class Platform;


namespace Application {

constexpr char mm6PathOverrideKey[] = "OPENENROTH_MM6_PATH";
constexpr char mm7PathOverrideKey[] = "OPENENROTH_MM7_PATH";
constexpr char mm8PathOverrideKey[] = "OPENENROTH_MM8_PATH";

std::string resolveMm6Path(Platform *platform);
std::string resolveMm7Path(Platform *platform);
std::string resolveMm8Path(Platform *platform);

}; // namespace Application
