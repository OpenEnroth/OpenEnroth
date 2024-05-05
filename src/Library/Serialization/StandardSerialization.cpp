#include "StandardSerialization.h"

#include <fast_float/fast_float.h>

#include <string>
#include <charconv>

#include "Utility/String/Format.h"

#include "SerializationExceptions.h"

namespace detail {
template<class T>
const char *typeName();

#define MM_DEFINE_TYPE_NAME(T) template<> const char *typeName<T>() { return #T; }
MM_DEFINE_TYPE_NAME(short)
MM_DEFINE_TYPE_NAME(unsigned short)
MM_DEFINE_TYPE_NAME(int)
MM_DEFINE_TYPE_NAME(unsigned int)
MM_DEFINE_TYPE_NAME(long)
MM_DEFINE_TYPE_NAME(unsigned long)
MM_DEFINE_TYPE_NAME(long long)
MM_DEFINE_TYPE_NAME(unsigned long long)
MM_DEFINE_TYPE_NAME(float)
MM_DEFINE_TYPE_NAME(double)
#undef MM_DEFINE_TYPE_NAME
} // namespace detail

// bool

bool trySerialize(const bool &src, std::string *dst) {
    *dst = src ? "true" : "false";
    return true;
}

bool tryDeserialize(std::string_view src, bool *dst) {
    if (src == "true" || src == "1") {
        *dst = true;
        return true;
    } else if (src == "false" || src == "0") {
        *dst = false;
        return true;
    } else {
        return false;
    }
}

void serialize(const bool &src, std::string *dst) {
    (void) trySerialize(src, dst); // Always succeeds.
}

void deserialize(std::string_view src, bool *dst) {
    if (!tryDeserialize(src, dst))
        throwDeserializationError(src, "bool");
}

// std::string

bool trySerialize(const std::string &src, std::string *dst) {
    *dst = src;
    return true;
}

bool tryDeserialize(std::string_view src, std::string *dst) {
    *dst = std::string(src);
    return true;
}

void serialize(const std::string &src, std::string *dst) {
    (void) trySerialize(src, dst);
}

void deserialize(std::string_view src, std::string *dst) {
    (void) tryDeserialize(src, dst);
}

// floats

namespace detail_float {
template<class T>
inline bool trySerialize(const T &src, std::string *dst) {
    *dst = fmt::format("{}"_cf, src);
    return true;
}

template<class T>
inline bool tryDeserialize(std::string_view src, T *dst) {
    const char *end = src.data() + src.size();
    fast_float::from_chars_result result = fast_float::from_chars(src.data(), end, *dst);
    return result.ec == std::errc() && result.ptr == end;
}

template<class T>
inline void serialize(const T &src, std::string *dst) {
    (void) trySerialize(src, dst);
}

template<class T>
inline void deserialize(std::string_view src, T *dst) {
    const char *end = src.data() + src.size();
    fast_float::from_chars_result result = fast_float::from_chars(src.data(), end, *dst);

    if (result.ec != std::errc())
        throwNumberDeserializationError(src, detail::typeName<T>(), result.ec);

    if (result.ptr != end)
        throwNumberDeserializationError(src, detail::typeName<T>(), std::errc::invalid_argument); // "Not a number"
}
} // namespace detail_float

// integrals

namespace detail_integral {
template<class T>
inline bool trySerialize(const T &src, std::string *dst) {
    dst->resize(20);
    std::to_chars_result result = std::to_chars(dst->data(), dst->data() + dst->size(), src);
    assert(result.ec == std::errc()); // Should never fail.
    dst->resize(result.ptr - dst->data());
    return true;
}

template<class T>
inline bool tryDeserialize(std::string_view src, T *dst) {
    const char *end = src.data() + src.size();
    std::from_chars_result result = std::from_chars(src.data(), end, *dst);
    return result.ec == std::errc() && result.ptr == end;
}

template<class T>
inline void serialize(const T &src, std::string *dst) {
    (void) trySerialize(src, dst);
}

template<class T>
inline void deserialize(std::string_view src, T *dst) {
    const char *end = src.data() + src.size();
    std::from_chars_result result = std::from_chars(src.data(), end, *dst);

    if (result.ec != std::errc())
        throwNumberDeserializationError(src, detail::typeName<T>(), result.ec);

    if (result.ptr != end)
        throwNumberDeserializationError(src, detail::typeName<T>(), std::errc::invalid_argument); // "Not a number"
}
} // namespace detail_integral

#define MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(TYPE, NAMESPACE)                                                        \
    bool trySerialize(const TYPE &src, std::string *dst) { return NAMESPACE::trySerialize(src, dst); }                  \
    bool tryDeserialize(std::string_view src, TYPE *dst) { return NAMESPACE::tryDeserialize(src, dst); }                \
    void serialize(const TYPE &src, std::string *dst) { NAMESPACE::serialize(src, dst); }                               \
    void deserialize(std::string_view src, TYPE *dst) { NAMESPACE::deserialize(src, dst); }

MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(short, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(unsigned short, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(int, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(unsigned int, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(long, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(unsigned long, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(long long, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(unsigned long long, detail_integral)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(float, detail_float)
MM_DEFINE_PROXY_SERIALIZATION_FUNCTIONS(double, detail_float)
