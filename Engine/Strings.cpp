#include <stdarg.h>
#include <vector>
#include <algorithm>

#include "Engine/Strings.h"

String StringFromInt(int value) { return StringPrintf("%d", value); }

String StringPrintf(const char *fmt, ...) {
    int size = 1024;
    char *buffer = new char[size];

    va_list va;
    va_start(va, fmt);
    {
        int nsize = vsnprintf(buffer, size, fmt, va);
        if (size <= nsize) {  // fail delete buffer and try again
            delete[] buffer;
            buffer = new char[nsize + 1];  // +1 for \0
            nsize = vsnprintf(buffer, size, fmt, va);
        }
    }
    va_end(va);

    String ret(buffer);
    delete[] buffer;
    return ret;
}

std::vector<char *> Tokenize(char *input, const char separator) {
    std::vector<char *> retVect;
    retVect.push_back(input);
    while (*input) {
        if (*input == separator) {
            *input = '\0';
            retVect.push_back(input + 1);
        }
        ++input;
    }
    return retVect;
}

String MakeLower(const String &text) {
    std::string tmp = text;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    return tmp;
}
