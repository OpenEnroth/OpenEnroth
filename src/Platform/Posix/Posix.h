#pragma once
#include <unistd.h>

#include <iostream>

#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _mbsncmp(str1, str2, maxCount) strncmp((const char*)str1, (const char*)str2, maxCount)

inline void _strlwr(char *src) {
    char* it = src;
    for (; *it; ++it)
        *it = tolower(*it);
}

#include <string.h>
inline char *_strrev(char *str) {
    if (!str || !*str)
        return str;

    char ch;
    int i = strlen(str) - 1, j = 0;
    while (i > j) {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

#define _flushall() std::cout.flush()

#define _access access  // TODO: this should really be isolated behind OS_ API
