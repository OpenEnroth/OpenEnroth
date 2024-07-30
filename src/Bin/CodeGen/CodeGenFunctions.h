#pragma once

#include <cstdio>
#include <cctype>
#include <vector>
#include <array>
#include <string_view>
#include <string>
#include <algorithm>

#include "Utility/String/Format.h"

inline std::string toUpperCaseEnum(std::string_view string) {
    std::string result;
    for (char c : trim(string)) {
        if (std::isalnum(c)) {
            result += static_cast<char>(toupper(c));
        } else if (std::isspace(c) || c == '/' || c == '-' || c == '_') {
            if (!result.ends_with('_'))
                result += '_';
        }
    }
    return result;
}

template<size_t N>
void dumpAligned(FILE *file, std::string_view prefix, const std::vector<std::array<std::string, N>> &table) {
    std::array<size_t, N> maxLengths;
    maxLengths.fill(0);

    for (const auto &line : table)
        for (size_t i = 0; i < line.size(); i++)
            maxLengths[i] = std::max(maxLengths[i], line[i].size());

    for (const auto &line : table) {
        std::string output = std::string(prefix);
        for (size_t i = 0; i < line.size(); i++) {
            output += line[i];
            output += std::string(maxLengths[i] - line[i].size(), ' ');
        }
        while (output.ends_with(' ')) // Note that we don't trim front.
            output.pop_back();
        fmt::println(file, "{}", output);
    }
}
