#pragma once

#include <string>
#include <utility>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/StringOutputStream.h"
#include "Utility/Memory/Blob.h"

#include "BinaryConcepts.h"

// Blob is a proxy binary serialization type, and we need to tell other overloads not to handle it.
// This way overloads for Blob will always come first in the call chain, and this is what we want.
template<>
struct is_proxy_binary_serialization_source<Blob> : std::true_type {};
template<>
struct is_proxy_binary_serialization_target<Blob> : std::true_type {};

template<class Src, class... Tag>
void serialize(const Src &src, Blob *dst, const Tag &... tag) {
    std::string tmp;
    StringOutputStream stream(&tmp);
    serialize(src, &stream, tag...);
    *dst = Blob::fromString(std::move(tmp));
}

template<class Dst, class... Tag>
void deserialize(const Blob &src, Dst *dst, const Tag &... tag) {
    // Using MemoryInputStream and not BlobInputStream is intentional. BlobInputStream is heavier, and we don't need
    // its functionality here.
    MemoryInputStream stream(src.data(), src.size());
    deserialize(stream, dst, tag...);
    // TODO(captainurist): check that there's no data left in the stream.
}
