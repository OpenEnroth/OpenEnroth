#pragma once

#include <string>

namespace mINI {
template<class T>
class INIMap;
using INIStructure = INIMap<INIMap<std::string>>;
} // namespace mINI

class Config;
class ConfigSection;
template<class T>
class ConfigValue;

template<class T>
void RegisterConfigValue(ConfigValue<T> *value);
