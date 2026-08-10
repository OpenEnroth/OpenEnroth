#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/SmallVector.h"
#include "Utility/View.h"

#include "TsvLine.h"

class Blob;

/**
 * Reader for the tab-separated tables that MS Excel writes out. The format is not the same as a plain split on
 * tabs and newlines:
 * - Rows are separated by CRLF, and cells inside a row by a tab.
 * - A cell can contain newlines, and those are bare LFs.
 * - Excel wraps a cell in double quotes if it contains a newline or a double quote, and the double quotes inside
 *   such a cell are then written out doubled.
 *
 * Cells come out trimmed and unquoted.
 *
 * Lines are read lazily and the buffers are reused between lines, so iterating a table doesn't allocate.
 *
 * Every row is read, including the empty ones - use `skip(&TsvLine::isEmpty)` to drop those.
 *
 * Inherits the fluent view methods, so `take`, `zip` and friends work on the row range.
 */
class TsvReader : public ViewInterface<TsvReader> {
 public:
    TsvReader() = default; // An empty table. Needed because the view adaptors store the reader by value.

    /**
     * @param data                      Table contents, as written out by Excel. Not copied, so it must outlive the
     *                                  reader.
     * @param displayPath               Path to the table, used in error messages.
     */
    explicit TsvReader(std::string_view data, std::string_view displayPath = {});

    /**
     * Same as above, taking the display path from the blob.
     *
     * @param data                      Table contents. Not copied, so the blob must outlive the reader - which is
     *                                  why there is no overload for a temporary.
     */
    explicit TsvReader(const Blob &data);
    explicit TsvReader(Blob &&data) = delete;

    class Iterator;
    class Sentinel {};

    [[nodiscard]] Iterator begin();
    [[nodiscard]] Sentinel end() const { return {}; }

    [[nodiscard]] std::string_view displayPath() const { return _displayPath; }

 private:
    /**
     * Reads the next line into `_line` and `_cells`.
     *
     * @return                          Whether a line was read.
     */
    bool readLine();

    /**
     * Unquotes a cell using CSV quoting rules - a double-quoted cell has its quotes stripped, and doubled quotes
     * inside it are un-doubled. Takes a scratch entry only for the cells that need un-doubling.
     *
     * @param str                       Cell to unquote.
     * @return                          View into `str`, or into `_scratch` if the cell needed un-doubling.
     */
    std::string_view unquote(std::string_view str);

 private:
    friend class TsvCell;
    friend class TsvLine;

    std::string_view _data;
    std::string _displayPath;

    size_t _pos = 0; // Offset of the next line in `_data`.
    size_t _lineNumber = 0; // 1-based number of the current line.
    std::string_view _line;
    bool _empty = false; // Whether all cells of the current line are empty.
    gch::small_vector<std::string_view, 64> _cells;
    // Holds the unquoted text of the cells that needed un-doubling, which is a rarity. A deque because
    // references into it must survive the pushes that happen while the rest of the line is parsed.
    std::deque<std::string> _scratch;
    size_t _scratchUsed = 0; // Entries of `_scratch` used by the current line, the rest are reused as is.
};

class TsvReader::Iterator {
 public:
    using value_type = TsvLine;
    using difference_type = ptrdiff_t;

    Iterator() = default;
    explicit Iterator(TsvReader *reader) : _reader(reader) {}

    [[nodiscard]] TsvLine operator*() const { return TsvLine(_reader); }

    Iterator &operator++() {
        if (!_reader->readLine())
            _reader = nullptr;
        return *this;
    }

    void operator++(int) { ++*this; }

    [[nodiscard]] friend bool operator==(const Iterator &l, Sentinel) { return l._reader == nullptr; }

 private:
    TsvReader *_reader = nullptr;
};

inline TsvCell TsvLine::operator[](size_t column) const {
    std::string_view value = column < _reader->_cells.size() ? _reader->_cells[column] : std::string_view();
    return TsvCell(value, _reader, column);
}

inline size_t TsvLine::size() const {
    return _reader->_cells.size();
}

inline size_t TsvLine::lineNumber() const {
    return _reader->_lineNumber;
}

inline std::string_view TsvLine::str() const {
    return _reader->_line;
}

inline auto TsvLine::cells() const {
    return view(_reader->_cells);
}

inline bool TsvLine::isEmpty() const {
    return _reader->_empty;
}
