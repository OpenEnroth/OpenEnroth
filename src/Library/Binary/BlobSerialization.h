#pragma once

#include <string>
#include <utility>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/StringOutputStream.h"
#include "Utility/Memory/Blob.h"

template<class Src, class... Tag> requires (sizeof...(Tag) <= 1)
void serialize(const Src &src, Blob *dst, const Tag &... tag) {
    std::string tmp;
    StringOutputStream stream(&tmp);
    serialize(src, &stream, tag...);
    *dst = Blob::fromString(std::move(tmp));
}

template<class Dst, class... Tag> requires (sizeof...(Tag) <= 1)
void deserialize(const Blob &src, Dst *dst, const Tag &... tag) {
    MemoryInputStream stream(src.data(), src.size());
    deserialize(stream, dst, tag...);
}
