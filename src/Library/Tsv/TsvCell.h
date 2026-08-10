#pragma once

#include <cstddef>
#include <exception>
#include <string>
#include <string_view>

#include "Library/Serialization/Serialization.h"

class TsvReader;

/**
 * A single cell of a `TsvLine`.
 *
 * Is a `std::string_view`, so reading a text cell is just `foo = line[3]`. Everything that needs parsing goes
 * through `as`, which reports the position of the cell in the file if it doesn't parse.
 *
 * On why this derives from `std::string_view` instead of just converting to it. A cell also has to convert to
 * `std::string`, so that `std::string s = line[3]` works. And `f(std::string_view)` plus `f(std::string &&)` is a
 * perfectly sane overload set that we want to keep working - `split` is exactly that, with the string overload
 * deleted so that splitting a temporary doesn't dangle. Had the cell merely converted to both types, such a call
 * would be ambiguous, because the two conversion sequences are both user-defined and thus rank the same.
 * Deriving makes the conversion to `std::string_view` a derived-to-base one, which is a standard conversion, and
 * standard beats user-defined - so the view overload wins, and the overload set keeps working.
 */
class TsvCell : public std::string_view {
 public:
    operator std::string() const { // NOLINT: implicit, so that `std::string s = line[3]` works.
        return std::string(*this);
    }

    /**
     * @return                          Cell value parsed as `T` via `fromString`.
     * @throws Exception                If the cell doesn't parse.
     */
    template<class T>
    [[nodiscard]] T as() const {
        try {
            return fromString<T>(*this);
        } catch (const std::exception &e) {
            throwParseError(e.what());
        }
    }

 private:
    friend class TsvLine;

    // Mutating a cell in place makes no sense, so these are hidden. Note that a `std::string_view &` still gets to
    // them, a base class member cannot really be taken away.
    using std::string_view::remove_prefix;
    using std::string_view::remove_suffix;
    using std::string_view::swap;

    TsvCell(std::string_view value, const TsvReader *reader, size_t column) :
        std::string_view(value), _reader(reader), _column(column) {}

    [[noreturn]] void throwParseError(std::string_view message) const;

 private:
    const TsvReader *_reader = nullptr;
    size_t _column = 0;
};
