#pragma once

#include <functional> // For std::hash.
#include <limits>
#include <utility>

namespace detail {

template <typename T>
void hashCombine(std::size_t &seed, const T &v) {
    // Borrowed from Boost.ContainerHash
    // https://github.com/boostorg/container_hash/blob/ee5285bfa64843a11e29700298c83a37e3132fcd/include/boost/container_hash/hash.hpp#L471
    //
    // Note that boost switched to this algo in 1.81, and the previous algorithm was questionable. See this thread:
    // https://stackoverflow.com/questions/35985960/c-why-is-boosthash-combine-the-best-way-to-combine-hash-values
    static constexpr int digits = std::numeric_limits<std::size_t>::digits;
    static_assert(digits == 64 || digits == 32);

    if constexpr (digits == 64) {
        // https://github.com/boostorg/container_hash/blob/ee5285bfa64843a11e29700298c83a37e3132fcd/include/boost/container_hash/detail/hash_mix.hpp#L67
        std::size_t x = seed + 0x9e3779b9 + std::hash<T>()(v);
        const std::size_t m = 0xe9846af9b1a615d;
        x ^= x >> 32;
        x *= m;
        x ^= x >> 32;
        x *= m;
        x ^= x >> 28;
        seed = x;
    } else { // 32-bits
        // https://github.com/boostorg/container_hash/blob/ee5285bfa64843a11e29700298c83a37e3132fcd/include/boost/container_hash/detail/hash_mix.hpp#L88
        std::size_t x = seed + 0x9e3779b9 + std::hash<T>()(v);
        const std::size_t m1 = 0x21f0aaad;
        const std::size_t m2 = 0x735a2d97;
        x ^= x >> 16;
        x *= m1;
        x ^= x >> 15;
        x *= m2;
        x ^= x >> 15;
        seed = x;
    }
}

} // namespace detail

template <class A, class B>
struct std::hash<std::pair<A, B>> {
    size_t operator()(const std::pair<A, B>& value) const {
        std::size_t seed = 0;
        detail::hashCombine(seed, value.first);
        detail::hashCombine(seed, value.second);
        return seed;
    }
};
