#pragma once

#include <cstddef>
#include <string_view>

#include "TsvCell.h"

class TsvReader;

/**
 * A single line of a `TsvReader`.
 *
 * Cells are trimmed and unquoted, and reading past the last cell yields an empty cell instead of throwing, so
 * ragged tables need no special handling.
 *
 * A `TsvLine`, and everything obtained from it, is invalidated when the reader advances. Copy the cells you need to
 * keep into `std::string`s.
 *
 * The accessors are defined in `TsvReader.h`, they need the reader to be a complete type.
 */
class TsvLine {
 public:
    [[nodiscard]] TsvCell operator[](size_t column) const;

    /**
     * @return                          Number of cells in this line. Reading past this is fine, and yields empty
     *                                  cells.
     */
    [[nodiscard]] size_t size() const;

    /**
     * @return                          1-based number of this line in the file, counting the empty lines.
     */
    [[nodiscard]] size_t lineNumber() const;

    /**
     * @return                          The whole line, as it appears in the file.
     */
    [[nodiscard]] std::string_view str() const;

    /**
     * @return                          View over all cells of this line, for the cases that positional access
     *                                  doesn't fit, e.g. `line.cells().drop(1).zip(someRange)`.
     */
    [[nodiscard]] auto cells() const;

    /**
     * Note that this is not the same as having no cells - a line of nothing but tabs has plenty of cells, and all of
     * them are empty.
     *
     * @return                          Whether all cells of this line are empty. Excel pads a table with runs of
     *                                  tabs past the last data row, and `skip(&TsvLine::isBlank)` drops those.
     */
    [[nodiscard]] bool isBlank() const;

 private:
    friend class TsvReader;

    explicit TsvLine(const TsvReader *reader) : _reader(reader) {}

 private:
    const TsvReader *_reader = nullptr;
};
