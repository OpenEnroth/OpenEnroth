#pragma once

#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

#ifndef _WINDOWS
#include "Platform/Posix/Posix.h"
#endif

std::string StringPrintf(const char *fmt, ...);
std::string StringFromInt(int value);
std::string ToLower(std::string_view text);
std::string ToUpper(std::string_view text);
std::vector<char*> Tokenize(char* input, const char separator);

//----- (00452C30) --------------------------------------------------------
inline char *RemoveQuotes(char *str) {
    if (*str == '"') {
        str[strlen(str) - 1] = 0;
        return str + 1;
    }
    return str;
}

inline std::string TrimRemoveQuotes(std::string str) {
    while (str.length() > 0 && (str.at(0) == ' ' || str.at(0) == '"')) {
        str.erase(0, 1);
    }
    while (str.length() > 0 && (str.at(str.length() - 1) == ' ' || str.at(str.length() - 1) == '"')) {
        str.pop_back();
    }

    return str;
}

inline bool iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;

    char diff = 0;
    for (size_t i = 0, length = a.size(); i < length; i++)
        diff |= (a[i] ^ b[i]);
    return (diff == 0) || (diff == 32);
}

inline bool iless(std::string_view a, std::string_view b) {
    int result = _strnicmp(a.data(), b.data(), std::min(a.size(), b.size()));
    if (result < 0)
        return true;
    if (result > 0)
        return false;
    return a.size() < b.size();
}

struct ILess {
    bool operator()(std::string_view a, std::string_view b) const {
        return iless(a, b);
    }
};

