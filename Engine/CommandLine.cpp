#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

#include "Engine/CommandLine.h"

#include "Engine/Strings.h"

using Engine_::CommandLine;

static inline bool starts_with(const std::string& haystack, const std::string& needle) {
    auto i = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char c1, char c2) {
            return std::toupper(c1) == std::toupper(c2);
        }
    );

    return i != haystack.end();
}

static inline std::vector<std::string> split(const std::string& text, char sep) {
    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    if (!text.empty()) {
        tokens.push_back(text.substr(start));
    }
    return tokens;
}

static inline void ltrim(std::string& s) {
    s.erase(
        s.begin(),
        std::find_if(
            s.begin(),
            s.end(),
            [](int ch) {
                return !std::isspace(ch);
            }
        )
    );
}

static inline void rtrim(std::string& s) {
    s.erase(
        std::find_if(
            s.rbegin(),
            s.rend(),
            [](int ch) {
                return !std::isspace(ch);
            }
        ).base(),
        s.end()
    );
}

static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}


CommandLine::CommandLine(const std::string& command_line) {
    this->command_line_tokens = split(command_line, ' ');
    for (auto token : command_line_tokens) {
        trim(token);
    }
}

bool CommandLine::TryFindKey(const std::string& key) const {
    for (auto token : command_line_tokens) {
        if (iequals(token, key)) {
            return true;
        }
    }
    return false;
}

bool CommandLine::TryGetValue(const std::string& key, std::shared_ptr<std::string>* out_value) const {
    for (auto token : command_line_tokens) {
        if (starts_with(token, key)) {
            if (out_value != nullptr) {
                auto key_value_pair = split(token, '=');
                if (iequals(key_value_pair[0], key) && key_value_pair.size() > 1) {
                    std::string value = key_value_pair[1];
                    trim(value);
                    if (value.empty()) {
                        return false;
                    }

                    *out_value = std::make_shared<std::string>(value);
                } else {
                    *out_value = std::make_shared<std::string>("");
                }
            }
            return true;
        }
    }
    return false;
}
