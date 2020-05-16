#pragma once

#include <string>
#include <functional>
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

inline bool iequals(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](const unsigned char &a, const unsigned char &b) {
                          return tolower(a) < tolower(b);
                      }
    );
}

inline auto iequals_functor = std::function <bool (const std::string&, const std::string&)>(iequals);
