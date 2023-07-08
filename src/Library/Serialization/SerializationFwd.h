#pragma once

#include <string>
#include <string_view>

/**
 * Forward-declares lexical serialization functions for the given type.
 *
 * @param TYPE                          Type to forward-declare lexical serialization functions for.
 */
#define MM_DECLARE_SERIALIZATION_FUNCTIONS(TYPE)                                                                        \
    [[nodiscard]] bool trySerialize(const TYPE &src, std::string *dst);                                                 \
    [[nodiscard]] bool tryDeserialize(std::string_view src, TYPE *dst);                                                 \
    void serialize(const TYPE &src, std::string *dst);                                                                  \
    void deserialize(std::string_view src, TYPE *dst);                                                                  \
    inline bool lexical_cast(const std::string& src, TYPE& dst) { /* CLI11 support. */                                  \
        return tryDeserialize(src, &dst);                                                                               \
    }
