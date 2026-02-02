#pragma once

#include <string>
#include <utility>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Memory/Blob.h"

#include "BinaryConcepts.h"

template<class Src, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
void serialize(const Src &src, Blob *dst, const Tags &... tags) {
    BlobOutputStream stream(dst);
    serialize(src, &stream, tags...);
    stream.close(); // Flush data into a Blob.
}

template<class Dst, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
void deserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    // Using MemoryInputStream and not BlobInputStream is intentional. BlobInputStream is heavier, and we don't need
    // its functionality here.
    MemoryInputStream stream(src.data(), src.size());
    deserialize(stream, dst, tags...);
    // TODO(captainurist): check that there's no data left in the stream.
}
