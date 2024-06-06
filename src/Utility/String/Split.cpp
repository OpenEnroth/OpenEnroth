#include "Split.h"

#include <string_view>
#include <vector>

std::vector<char *> tokenize(char *input, const char separator) {
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

void split(std::string_view s, char sep, std::vector<std::string_view> *result) {
    result->clear();
    result->reserve(16);
    for (std::string_view chunk : split(s, sep))
        result->push_back(chunk);
}
