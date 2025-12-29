#include "Split.h"

#include <string_view>
#include <vector>

void split(std::string_view s, char sep, std::vector<std::string_view> *result) {
    result->clear();
    result->reserve(16);
    for (std::string_view chunk : split(s, sep))
        result->push_back(chunk);
}
