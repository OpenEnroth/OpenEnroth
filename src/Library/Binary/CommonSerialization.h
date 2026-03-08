#pragma once

#include <cassert>
#include <span>
#include <array>
#include <typeinfo>
#include <string>

#include "BinaryFwd.h"
#include "BinaryTags.h"
#include "BinaryConcepts.h"
#include "BinaryExceptions.h"

#include "Utility/Streams/InputStream.h"
#include "Utility/Streams/OutputStream.h"

namespace detail {
template<class Container>
struct AppendWrapper {
 public:
    using value_type = typename Container::value_type;

    explicit AppendWrapper(Container *container) : _container(container), _initialSize(container->size()) {}

    size_t size() const {
        return _container->size() - _initialSize;
    }

    auto data() const {
        return _container->data() + _initialSize;
    }

    void resize(size_t size) {
        _container->resize(_initialSize + size);
    }

 private:
    Container *_container;
    size_t _initialSize = 0;
};
} // namespace detail


//
// Serialization for memcopy-able types.
//

template<class T> requires is_memcopy_serializable_v<T>
void serialize(const T &src, OutputStream *dst) {
    dst->write(&src, sizeof(T));
}

template<class T> requires is_memcopy_serializable_v<T>
void deserialize(InputStream &src, T *dst) {
    size_t bytes = src.read(dst, sizeof(T));
    if (bytes != sizeof(T))
        throwBinarySerializationNoMoreDataError(bytes, sizeof(T), typeid(T).name());
}


//
// std::span support - doesn't write size to the stream.
//

template<StdSpan Span, class T = typename Span::value_type>
void deserialize(InputStream &src, Span *dst) {
    if constexpr (is_memcopy_serializable_v<T>) {
        size_t bytesExpected = dst->size() * sizeof(T);
        size_t bytesRead = src.read(dst->data(), bytesExpected);
        if (bytesRead != bytesExpected)
            throwBinarySerializationNoMoreDataError(bytesRead % sizeof(T), sizeof(T), typeid(T).name());
    } else {
        for (T &element : *dst)
            deserialize(src, &element);
    }
}

template<StdSpan Span, class T = typename Span::value_type>
void serialize(const Span &src, OutputStream *dst) {
    if constexpr (is_memcopy_serializable_v<T>) {
        dst->write(src.data(), src.size() * sizeof(T));
    } else {
        for (const T &element : src)
            serialize(element, dst);
    }
}


//
// std::span support with tag forwarding - doesn't write size to the stream.
//

template<StdSpan Span, class... Tags>
void deserialize(InputStream &src, Span *dst, EachTag, const Tags &... tags) {
    for (auto &element : *dst)
        deserialize(src, &element, tags...);
}

template<StdSpan Span, class... Tags>
void serialize(const Span &src, OutputStream *dst, EachTag, const Tags &... tags) {
    for (const auto &element : src)
        serialize(element, dst, tags...);
}


//
// std::array support - doesn't write size to the stream.
//

template<class T, size_t N, class... Tags>
void serialize(const std::array<T, N> &src, OutputStream *dst, const Tags &... tags) {
    serialize(std::span(src), dst, tags...);
}

template<class T, size_t N, class... Tags>
void deserialize(InputStream &src, std::array<T, N> *dst, const Tags &... tags) {
    std::span span(*dst);
    deserialize(src, &span, tags...);
}


//
// std::vector support - writes size to the stream, unless this is changed with tags.
//

template<ResizableContiguousContainer Src, class... Tags>
void serialize(const Src &src, OutputStream *dst, const Tags &... tags) {
    assert(src.size() <= UINT32_MAX);

    uint32_t size = src.size();
    serialize(size, dst);
    std::span span(src.data(), src.size());
    serialize(span, dst, tags...);
}

template<ResizableContiguousContainer Dst, class... Tags>
void deserialize(InputStream &src, Dst *dst, const Tags &... tags) {
    uint32_t size;
    deserialize(src, &size);

    // TODO(captainurist): can we do this better?
    // Best-effort check - number of records required can't be larger than the number of bytes in the stream.
    if (size > src.size() - src.position())
        throwBinarySerializationNoMoreDataError(src.size() - src.position(), size, typeid(typename Dst::value_type).name());

    dst->resize(size);
    std::span span(dst->data(), dst->size());
    deserialize(src, &span, tags...);
}

template<ResizableContiguousContainer Src, class... Tags>
void serialize(const Src &src, OutputStream *dst, UnsizedTag, const Tags &... tags) {
    serialize(std::span(src), dst, tags...);
}

template<ResizableContiguousContainer Dst, class... Tags>
void deserialize(InputStream &src, Dst *dst, PresizedTag tag, const Tags &... tags) {
    dst->resize(tag.size);
    std::span span(dst->data(), dst->size());
    deserialize(src, &span, tags...);
}

template<ResizableContiguousContainer Dst, class... Tags>
void deserialize(InputStream &src, Dst *dst, AppendTag, const Tags &... tags) {
    detail::AppendWrapper wrapper(dst);
    deserialize(src, &wrapper, tags...);
}


//
// Serialization for null-terminated strings.
//

inline void deserialize(InputStream &src, std::string *dst, NullTerminatedTag) {
    *dst = src.readUntil('\0');
}
