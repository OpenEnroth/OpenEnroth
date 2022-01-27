#pragma once

#include <string>
#include <algorithm>
#include <cstring>
#ifndef _WINDOWS
#include "Platform/Lin/Lin.h"
#endif

typedef std::string String;

String StringPrintf(const char *fmt, ...);
String StringFromInt(int value);
String MakeLower(const String &text);

//----- (00452C30) --------------------------------------------------------
inline char *RemoveQuotes(char *str) {
    if (*str == '"') {
        str[strlen(str) - 1] = 0;
        return str + 1;
    }
    return str;
}

inline bool iequals(std::string_view a, std::string_view b) {
    return a.size() == b.size() && _strnicmp(a.data(), b.data(), a.size()) == 0;
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

