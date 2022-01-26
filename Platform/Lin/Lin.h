#pragma once
#include <unistd.h>

#include <iostream>

#if defined(_M_IX86) || defined(__i386__) || defined(_X86_)
#define MM_ARCH_X86
#elif defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)
#define MM_ARCH_X64
#else
#error UNSUPPORTED PLATFORM
// #elif defined(_M_ARM) || defined(__arm__)
// #define MM_ARCH_ARM
// #elif defined (_M_ARM64) || defined(__aarch64__)
// #define MM_ARCH_ARM64
#endif

#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define vswprintf_s vswprintf
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

