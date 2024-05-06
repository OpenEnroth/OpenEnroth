#include "Transformations.h"

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>

#include "Format.h"

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

std::string toHexDump(std::string_view s, size_t groupSize) {
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (groupSize && i && i % groupSize == 0)
            result += " ";
        result += fmt::format("{:02X}", static_cast<uint8_t>(s[i]));
    }
    return result;
}

std::string replaceAll(std::string_view text, std::string_view what, std::string_view replacement) {
    assert(!what.empty());
    std::string result;

    size_t pos = 0;
    while(true) {
        size_t next = text.find(what, pos);
        result.append(text.substr(pos, next == std::string_view::npos ? std::string_view::npos : next - pos));
        if (next == std::string_view::npos)
            break;

        result.append(replacement);
        pos = next + what.size();
    }

    return result;
}

std::string replaceAll(std::string_view text, char what, char replacement) {
    std::string result;
    result = text;

    for (char &c : result)
        if (c == what)
            c = replacement;

    return result;
}

void split(std::string_view s, char sep, std::vector<std::string_view> *result) {
    result->clear();
    result->reserve(16);
    split(s, sep, [&result] (std::string_view part) {
        result->push_back(part);
    });
}
