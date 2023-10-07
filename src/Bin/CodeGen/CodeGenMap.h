#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>

#include "Utility/Workaround/ToUnderlying.h"
#include "Utility/Format.h"

#include "CodeGenFunctions.h"

class CodeGenMap {
 public:
    template<class Enum>
    void insert(Enum enumValue, const std::string &name, const std::string &comment) {
        int value = std::to_underlying(enumValue);
        assert(!_nameByValue.contains(value));

        if (name.empty()) {
            insert(enumValue, std::to_string(value), comment);
            return;
        }
        assert(!name.empty());

        _commentByValue[value] = comment;

        if (_valueByName.contains(name)) {
            int count = ++_countByName[name];
            if (count == 2)
                _nameByValue[_valueByName[name]] = name + "_1";

            _nameByValue[value] = name + "_" + std::to_string(count);
        } else {
            _nameByValue[value] = name;
            _valueByName[name] = value;
            _countByName[name] = 1;
        }
    }

    void dump(FILE *file, const std::string &prefix) {
        std::vector<std::array<std::string, 2>> linesAndComments;
        for (const auto &[value, name] : _nameByValue) {
            std::string comment = _commentByValue[value];
            if (!comment.empty())
                comment = "// " + comment;

            linesAndComments.push_back({fmt::format("{}{} = {}, ", prefix, name, value), comment});
        }

        dumpAligned(file, "    ", linesAndComments);
    }

 private:
    std::map<int, std::string> _nameByValue;
    std::unordered_map<int, std::string> _commentByValue;
    std::unordered_map<std::string, int> _valueByName;
    std::unordered_map<std::string, int> _countByName;
};
