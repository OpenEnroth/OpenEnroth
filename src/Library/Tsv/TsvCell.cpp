#include "TsvCell.h"

#include <string_view>

#include "Utility/Exception.h"

#include "TsvReader.h"

void TsvCell::throwParseError(std::string_view message) const {
    throw Exception("{}:{}:{}: {}", _reader->displayPath().empty() ? "<tsv>" : _reader->displayPath(),
                    _reader->_lineNumber, _column + 1, message);
}
