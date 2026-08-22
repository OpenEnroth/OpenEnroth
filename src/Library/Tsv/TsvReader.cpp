#include "TsvReader.h"

#include <string>
#include <utility>
#include <string_view>

#include "Utility/Memory/Blob.h"
#include "Utility/String/Transformations.h"

TsvReader::TsvReader(std::string_view data, std::string_view displayPath) :
    _data(data), _displayPath(displayPath) {}

TsvReader::TsvReader(const Blob &data) : TsvReader(data.str(), data.displayPath()) {}


TsvReader::Iterator TsvReader::begin() {
    Iterator result(this);
    if (!readLine())
        return Iterator();
    return result;
}

bool TsvReader::readLine() {
    if (_pos >= _data.size()) {
        _line = {};
        _cells.clear();
        return false;
    }

    size_t end = _data.find("\r\n", _pos);
    _line = _data.substr(_pos, end == std::string_view::npos ? std::string_view::npos : end - _pos);
    _pos = end == std::string_view::npos ? _data.size() : end + 2;
    _lineNumber++;

    _cells.clear();
    _scratchUsed = 0;
    bool empty = true;
    for (size_t cellStart = 0; ; ) {
        size_t cellEnd = _line.find('\t', cellStart);
        std::string_view cell = trim(_line.substr(cellStart, cellEnd - cellStart));

        cell = unquote(cell);

        empty = empty && cell.empty();
        _cells.push_back(cell);

        if (cellEnd == std::string_view::npos)
            break;
        cellStart = cellEnd + 1;
    }

    _empty = empty;
    return true;
}


std::string_view TsvReader::unquote(std::string_view str) {
    if (!str.starts_with('"'))
        return str; // Not a quoted cell, the `""` escaping doesn't apply.

    str = str.substr(1);
    if (str.ends_with('"'))
        str = str.substr(0, str.size() - 1);

    size_t pos = str.find("\"\"");
    if (pos == std::string_view::npos)
        return str; // No doubled quotes, so the inside of the quotes is already the answer.

    if (_scratchUsed == _scratch.size())
        _scratch.emplace_back();
    std::string *scratch = &_scratch[_scratchUsed++];

    scratch->assign(str.substr(0, pos)); // Everything before the first doubled quote is known to be clean.
    for (size_t i = pos; i < str.size(); i++) {
        scratch->push_back(str[i]);
        if (str[i] == '"' && i + 1 < str.size() && str[i + 1] == '"')
            i++; // Doubled quote, emit only one.
    }
    return *scratch;
}
