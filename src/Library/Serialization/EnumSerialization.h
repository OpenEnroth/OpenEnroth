#pragma once

#include <cassert>
#include <string>
#include <type_traits>
#include <functional> // This is required for magic_enum.hpp, but is not included by it...

#define MAGIC_ENUM_RANGE_MIN (-256)
#define MAGIC_ENUM_RANGE_MAX (255)
#include <magic_enum.hpp>

#include "Utility/Preprocessor.h"

#include "Serialization.h"
#include "SerializationExceptions.h"
#include "EnumSerializer.h"

/**
 * This macro generates lexical serialization functions for the provided enumeration type.. The typical use case is to
 * use it in the `cpp` file, and use `MM_DECLARE_SERIALIZATION_FUNCTIONS` in the header file.
 *
 * Note that it should be invoked from the namespace of the target enumeration type, otherwise argument-dependent
 * lookup won't find it, and unqualified calls to `serialize` / `trySerialize` / `deserialize` / `tryDeserialize`
 * won't work. `ToString` and `FromString` function objects also won't work.
 *
 * However, it might make sense to invoke this macro from several different namespaces to generate several sets of
 * serialization functions for the same type. In this case the serialization functions won't be discovered by ADL,
 * so you'll have to use qualified calls, but this is likely exactly what you'd want. The only downside is that
 * `ToString` / `FromString` global function objects won't work.
 *
 * Usage example:
 * ```
 * MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(Key, CASE_INSENSITIVE, {
 *     { Key::CapsLock, "Caps Lock" },
 *     { Key::CapsLock, "CapsLock" },    // Alternative string, used only for deserialization.
 *     { Key::CapsLock, "CPSLCK" },      // Can have several alternative strings.
 *     // ...
 * })
 * ```
 *
 * @param ENUM                          Enum type to generate lexical serialization functions for.
 * @param CASE_SENSITIVITY              Whether the mapping is case sensitive, either `CASE_SENSITIVE` or
 *                                      `CASE_INSENSITIVE`.
 * @param ...                           Initializer list of enum-string pairs. Can contain repeated values, in which
 *                                      case the 1st match will be used during serialization / deserialization.
 */
#define MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(ENUM, CASE_SENSITIVITY, ...)                                             \
    MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_I(ENUM, CASE_SENSITIVITY, MM_PP_CAT(globalEnumSerializer, __LINE__), __VA_ARGS__)

#define MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_I(ENUM, CASE_SENSITIVITY, SERIALIZER, ...)                               \
    /* Note: this is static initialization order fiasco-unsafe, so you're not expected to do any serialization in */    \
    /* startup code. This is not checked, things will just blow up. */                                                  \
    static const ::detail::EnumSerializer<ENUM> SERIALIZER = ::detail::EnumSerializer<ENUM>(CASE_SENSITIVITY, __VA_ARGS__); \
                                                                                                                        \
    static const ::detail::EnumSerializer<ENUM> &serializer(std::type_identity<ENUM>) {                                 \
        return SERIALIZER;                                                                                              \
    }                                                                                                                   \
                                                                                                                        \
    bool trySerialize(const ENUM &src, std::string *dst) {                                                              \
        return SERIALIZER.trySerialize(src, dst);                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    bool tryDeserialize(std::string_view src, ENUM *dst) {                                                              \
        return SERIALIZER.tryDeserialize(src, dst);                                                                     \
    }                                                                                                                   \
                                                                                                                        \
    MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_VIA_TRY_FUNCTIONS_I(ENUM)

#define MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_VIA_TRY_FUNCTIONS_I(TYPE)                                                \
    void serialize(const TYPE &src, std::string *dst) {                                                                 \
        if (!trySerialize(src, dst))                                                                                    \
            throwEnumSerializationError(src, #TYPE);                                                                    \
    }                                                                                                                   \
                                                                                                                        \
    void deserialize(std::string_view src, TYPE *dst) {                                                                 \
        if (!tryDeserialize(src, dst))                                                                                  \
            throwDeserializationError(src, #TYPE);                                                                      \
    }


/**
 * This macro provides a limited support for lexical serialization of `Flags<T>`. It can only be used after an
 * invocation of `MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS` for the underlying enum type.
 *
 * @param FLAGS                         Flags type to generate lexical serialization functions for.
 */
#define MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(FLAGS)                                                                  \
    static const bool MM_PP_CAT(globalFlagsCheck, __LINE__) = [] {                                                      \
        assert(serializer(std::type_identity<typename FLAGS::enumeration_type>()).isUsableWithFlags());                 \
        return true;                                                                                                    \
    }();                                                                                                                \
                                                                                                                        \
    bool trySerialize(const FLAGS &src, std::string *dst) {                                                             \
        return serializer(std::type_identity<typename FLAGS::enumeration_type>()).trySerialize(src, dst);               \
    }                                                                                                                   \
                                                                                                                        \
    bool tryDeserialize(std::string_view src, FLAGS *dst) {                                                             \
        return serializer(std::type_identity<typename FLAGS::enumeration_type>()).tryDeserialize(src, dst);             \
    }                                                                                                                   \
                                                                                                                        \
    MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_VIA_TRY_FUNCTIONS_I(FLAGS)


/**
 * Same as `MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS`, but using `magic_enum`.
 *
 * Note that `MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS` currently cannot be used with enums serialized with this macro.
 *
 * @param ENUM                          Enum type to generate lexical serialization functions for.
 */
#define MM_DEFINE_ENUM_MAGIC_SERIALIZATION_FUNCTIONS(ENUM)                                                              \
    bool trySerialize(const ENUM &src, std::string *dst) {                                                              \
        std::string_view result = magic_enum::enum_name(src);                                                           \
        if (result.empty())                                                                                             \
            return false;                                                                                               \
        *dst = std::string(result);                                                                                     \
        return true;                                                                                                    \
    }                                                                                                                   \
                                                                                                                        \
    bool tryDeserialize(std::string_view src, ENUM *dst) {                                                              \
        std::optional<ENUM> result = magic_enum::enum_cast<ENUM>(src);                                                  \
        if (!result.has_value())                                                                                        \
            return false;                                                                                               \
        *dst = *result;                                                                                                 \
        return true;                                                                                                    \
    }                                                                                                                   \
                                                                                                                        \
    MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS_VIA_TRY_FUNCTIONS_I(ENUM)

