#pragma once

#include <string>

// TODO(captainurist): Proper error checking here

inline void DeserializeConfigValue(const std::string &src, bool *dst) {
    if (src == "0" || src == "false") {
        *dst = false;
    } else {
        *dst = true;
    }
}

inline void DeserializeConfigValue(const std::string &src, float *dst) {
    *dst = std::stof(src);
}

inline void DeserializeConfigValue(const std::string &src, int *dst) {
    *dst = std::stoi(src);
}

inline void DeserializeConfigValue(const std::string &src, std::string *dst) {
    *dst = src;
}

inline void SerializeConfigValue(bool src, std::string *dst) {
    *dst = src ? "true" : "false";
}

inline void SerializeConfigValue(int src, std::string *dst) {
    *dst = std::to_string(src);
}

inline void SerializeConfigValue(float src, std::string *dst) {
    *dst = std::to_string(src);
}

inline void SerializeConfigValue(const std::string &src, std::string *dst) {
    *dst = src;
}
