#include <string>


class Platform;


namespace Application {

constexpr char mm6PathOverrideKey[] = "OPENENROTH_MM6_PATH";
constexpr char mm7PathOverrideKey[] = "OPENENROTH_MM7_PATH";
constexpr char mm8PathOverrideKey[] = "OPENENROTH_MM8_PATH";

std::string ResolveMm6Path(Platform *platform);
std::string ResolveMm7Path(Platform *platform);
std::string ResolveMm8Path(Platform *platform);

}; // namespace Application
