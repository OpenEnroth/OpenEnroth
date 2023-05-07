#pragma once

#include <string>
#include <utility>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/StringOutputStream.h"
#include "Utility/Memory/Blob.h"

template<class T, class... Ctx>
void serialize(const T &src, Blob *dst, Ctx &&... ctx) {
    std::string tmp;
    StringOutputStream stream(&tmp);
    serialize(src, &stream, std::forward<Ctx>(ctx)...);
    *dst = Blob::fromString(std::move(tmp));
}

template<class T, class... Ctx>
void deserialize(const Blob &src, T &&dst, Ctx &&... ctx) {
    MemoryInputStream stream(src.data(), src.size());
    deserialize(stream, std::forward<T>(dst), std::forward<Ctx>(ctx)...);
}
