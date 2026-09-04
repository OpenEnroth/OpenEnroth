#pragma once

#include <cstddef>
#include <string_view>

#include "Utility/View.h"

#include "TsvCell.h"

class TsvReader;

/**
 * A single line of a `TsvReader`, and a range of its cells.
 *
 * Cells are trimmed and unquoted, and reading past the last cell yields an empty cell instead of throwing, so
 * ragged tables need no special handling.
 *
 * Positional access is the common case, and the range is there for the tables that walk all cells instead of
 * indexing them - naming the loop variable after what it holds keeps this readable:
 * ```
 * for (TsvLine cells : TsvReader(npcDist).drop(2).skip(&TsvLine::isBlank))
 *     for (auto [cell, map] : cells.drop(1).zip(allMaps()))
 * ```
 *
 * A `TsvLine`, and everything obtained from it, is invalidated when the reader advances. Copy the cells you need to
 * keep into `std::string`s.
 *
 * The accessors are defined in `TsvReader.h`, they need the reader to be a complete type.
 */
class TsvLine : public ViewInterface<TsvLine> {
 public:
    class Iterator {
     public:
        using value_type = TsvCell;
        using difference_type = ptrdiff_t;

        Iterator() = default;
        Iterator(const TsvReader *reader, size_t column) : _reader(reader), _column(column) {}

        [[nodiscard]] TsvCell operator*() const;

        Iterator &operator++() {
            _column++;
            return *this;
        }

        Iterator operator++(int) {
            Iterator result = *this;
            ++*this;
            return result;
        }

        [[nodiscard]] friend bool operator==(const Iterator &l, const Iterator &r) = default;

     private:
        const TsvReader *_reader = nullptr;
        size_t _column = 0;
    };

    [[nodiscard]] TsvCell operator[](size_t column) const;

    [[nodiscard]] Iterator begin() const;
    [[nodiscard]] Iterator end() const;

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
