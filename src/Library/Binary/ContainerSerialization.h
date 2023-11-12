#pragma once

#include <cassert>
#include <span>
#include <array>

#include "MemCopySerialization.h"
#include "BinaryTags.h"
#include "BinaryConcepts.h"

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
// std::array support - doesn't write size to the stream.
//

template<class T, size_t N>
struct is_proxy_binarizable<std::array<T, N>> : std::true_type {}; // std::array forwards to std::span.

template<class T, size_t N, RegularBinarySink Dst, class... Tags>
void serialize(const std::array<T, N> &src, Dst *dst, const Tags &... tags) {
    serialize(std::span(src), dst, tags...);
}

template<RegularBinarySource Src, class T, size_t N, class... Tags>
void deserialize(Src &src, std::array<T, N> *dst, const Tags &... tags) {
    std::span span(*dst);
    deserialize(src, &span, tags...);
}


//
// std::vector support - writes size to the stream, unless this is changed with tags.
//

template<ResizableContiguousContainer Container>
struct is_proxy_binarizable<Container> : std::true_type {}; // ResizableContiguousContainer forwards to std::span.

template<ResizableContiguousContainer Src, RegularBinarySink Dst, class... Tags>
void serialize(const Src &src, Dst *dst, const Tags &... tags) {
    assert(src.size() <= UINT32_MAX);

    uint32_t size = src.size();
    serialize(size, dst);
    std::span span(src.data(), src.size());
    serialize(span, dst, tags...);
}

template<RegularBinarySource Src, ResizableContiguousContainer Dst, class... Tags>
void deserialize(Src &src, Dst *dst, const Tags &... tags) {
    uint32_t size;
    deserialize(src, &size);

    dst->resize(size);
    std::span span(dst->data(), dst->size());
    deserialize(src, &span, tags...);
}

template<ResizableContiguousContainer Src, RegularBinarySink Dst, class... Tags>
void serialize(const Src &src, Dst *dst, UnsizedTag, const Tags &... tags) {
    serialize(std::span(src), dst, tags...);
}

template<RegularBinarySource Src, ResizableContiguousContainer Dst, class... Tags>
void deserialize(Src &src, Dst *dst, PresizedTag tag, const Tags &... tags) {
    dst->resize(tag.size);
    std::span span(dst->data(), dst->size());
    deserialize(src, &span, tags...);
}

template<RegularBinarySource Src, ResizableContiguousContainer Dst, class... Tags>
void deserialize(Src &src, Dst *dst, AppendTag, const Tags &... tags) {
    detail::AppendWrapper wrapper(dst);
    deserialize(src, &wrapper, tags...);
}

