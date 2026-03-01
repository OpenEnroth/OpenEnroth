#pragma once

#include <type_traits>

#include "SnapshotTags.h"

template<class Src, class Dst, class Via, class... Tags>
void deserialize(Src &&src, Dst *dst, ViaTag<Via>, const Tags &... tags) {
    static_assert(!std::is_same_v<Via, Dst>, "Intermediate and target types must be different.");

    Via tmp;
    deserialize(src, &tmp);
    reconstruct(tmp, dst, tags...);
}

template<class Src, class Dst, class Via, class... Tags>
void serialize(const Src &src, Dst *dst, ViaTag<Via>, const Tags &... tags) {
    static_assert(!std::is_same_v<Via, Src>, "Intermediate and source types must be different.");

    Via tmp;
    snapshot(src, &tmp, tags...);
    serialize(tmp, dst);
}
