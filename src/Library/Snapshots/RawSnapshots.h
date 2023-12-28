#pragma once

#include "SnapshotConcepts.h"

//
// raw() protocol support, for exposing private fields to the snapshot code.
//

template<RawAccessible T1, class T2>
void snapshot(const T1 &src, T2 *dst) {
    snapshot(raw(src), dst);
}

template<class T1, RawAccessible T2>
void reconstruct(const T1 &src, T2 *dst) {
    reconstruct(src, &raw(*dst));
}


/**
 * Helper macro for classes that want to keep their members private, while allowing for external `snapshot` /
 * `reconstruct` functions.
 *
 * The way to make this work is:
 * 1. A class is split into two - `Class` and `RawClass`.
 * 2. `RawClass` exposes all the members as public fields.
 * 3. `Class` uses private inheritance to derive from `RawClass` and uses `MM_DECLARE_RAW_PRIVATE_BASE(RawClass)`
 *    inside the class definition.
 * 4. `snapshot` and `reconstruct` functions are written for the `RawClass`.
 * 5. Thanks to the overloads above, `snapshot` and `reconstruct` also work for `Class` by forwarding to the overloads
 *    for `RawClass`.
 *
 * Example code:
 * ```
 * struct RawTime {
 *     std::int64_t _time = 0;
 * };
 *
 * class Time : private RawTime {
 *     MM_DECLARE_RAW_PRIVATE_BASE(RawTime);
 * public:
 *     // Time implementation here.
 * };
 *
 * void snapshot(const RawTime &src, Time_MM7 *dst) {
 *     dst->time = src._time;
 * }
 *
 * // At this point you can call snapshot(Time, Time_MM7 *), and it will work as expected.
 * ```
 */
#define MM_DECLARE_RAW_PRIVATE_BASE(BASE)                                                                               \
 public:                                                                                                                \
    template<class Self>                                                                                                \
    friend auto &raw(Self &self) {                                                                                      \
        if constexpr (std::is_const_v<Self>) {                                                                          \
            return static_cast<const BASE &>(self);                                                                     \
        } else {                                                                                                        \
            return static_cast<BASE &>(self);                                                                           \
        }                                                                                                               \
    }                                                                                                                   \
 private:                                                                                                               \
    // NOLINT: blank line after private: is intentional.
