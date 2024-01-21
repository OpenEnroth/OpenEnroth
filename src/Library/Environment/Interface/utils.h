#pragma once

#include <string>

inline std::string getStringOr(const char* value, std::string default_value = "") {
    return value ? value
                 : default_value;
}
