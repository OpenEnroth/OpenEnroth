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
struct is_binary_serialization_proxy<Blob> : std::true_type {};

template<class Src, class... Tags>
void serialize(const Src &src, Blob *dst, const Tags &... tags) {
    std::string tmp;
    StringOutputStream stream(&tmp);
    serialize(src, &stream, tags...);
    *dst = Blob::fromString(std::move(tmp));
}

template<class Dst, class... Tags>
void deserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    // Using MemoryInputStream and not BlobInputStream is intentional. BlobInputStream is heavier, and we don't need
    // its functionality here.
    MemoryInputStream stream(src.data(), src.size());
    deserialize(stream, dst, tags...);
    // TODO(captainurist): check that there's no data left in the stream.
}
